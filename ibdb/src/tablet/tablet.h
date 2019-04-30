/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-12 21:22:21 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-15 16:07:23
 */

#ifndef IBDB_TABLET_TABLET_H
#define IBDB_TABLET_TABLET_H
#include <mutex>

#include "base/noncopyable.h"
#include "port/port.h"
#include "protobuf/rpc.pb.h"
#include "protobuf/storage.pb.h"
#include "storage/table.h"

using ibdb::base::Noncopyable;
using ibdb::storage::Table;
using ibdb::storage::Schema;
using ibdb::storage::TableManifest;
using ibdb::storage::Field;
using ibdb::port::RpcCode;
using ibdb::rpc::TabletManifest;


DECLARE_uint32(log_index_sparse_threshold);
DECLARE_string(db_root);
DECLARE_string(table_timestamp);

namespace ibdb {
namespace tablet {

class Tablet : Noncopyable {

public:
    // endpoint = ip + port
    // don't use ip and port sepaately
    Tablet(const std::string& endpoint, const bool is_leader) : endpoint_(endpoint), is_leader_(is_leader) {}
    explicit Tablet(const ibdb::rpc::TabletManifest& tablet_manifest);
    ~Tablet() {}
    bool Init();
    bool Create(const ::ibdb::rpc::CreateRequest* const request, ::ibdb::rpc::CreateResponse* const response);
    bool Put(const ::ibdb::rpc::PutRequest* const request, ::ibdb::rpc::PutResponse* const response);
    bool Get(const ::ibdb::rpc::GetRequest* const request, ::ibdb::rpc::GetResponse* const response);
    std::vector<std::string> GetStatements(const std::string& table_name, const uint64_t start_offset, const uint64_t end_offset);
    bool Delete();
    inline TableManifest* GetTableManifest(const std::string& table_name) {
        auto iterator = table_manifest_map_.find(table_name);
        if (iterator == table_manifest_map_.end()) {
            LOG(ERROR) << "table manifest is not found";
            return nullptr;
        }
        return &iterator->second;
    }

    inline TabletManifest GetTabletManifest() {
        return *tablet_manifest_;
    }
private:
    std::mutex mu_;
    std::string endpoint_;
    bool is_leader_;
    ibdb::rpc::TabletManifest* tablet_manifest_;
    // table name with table
    std::map<std::string, std::shared_ptr<Table>> table_map_;
    // table name with table manifest
    std::map<std::string, TableManifest> table_manifest_map_;
    // std::map<std::string, uint64_t> table_offset_map_;
};

Tablet::Tablet(const ibdb::rpc::TabletManifest& tablet_manifest) {
    tablet_manifest_->CopyFrom(tablet_manifest);
    endpoint_ = tablet_manifest_->endpoint();
    is_leader_ = tablet_manifest_->is_leader();
}

// you must be invoke Init() after invoking Tablet()
bool Tablet::Init() {
    // if (endpoint_ == nullptr) {
    //     return false;
    // }
    tablet_manifest_ = new ibdb::rpc::TabletManifest();
    tablet_manifest_->set_endpoint(endpoint_);
    tablet_manifest_->set_is_leader(is_leader_);
    return true;
}

// create table_name ts_name,uint_64,isIndex col_name,type,isIndex  col_name,type,isIndex
// assume request is vaild
bool Tablet::Create(const ::ibdb::rpc::CreateRequest* const request, ::ibdb::rpc::CreateResponse* const response) {
    std::string statement = request->statement();
    std::string delim(" ");
    std::vector<std::string> result;
    ibdb::base::SplitString(statement, delim, &result);
    std::string table_name = result.at(1);
    if (table_map_.find(table_name) != table_map_.end()) {
        return false;
    }
    Schema schema;
    delim = ",";
    for (int i = 2; i < result.size(); i++) {
        Field* field = schema.add_field();
        std::string key = result.at(i);
        std::vector<std::string> key_info;
        ibdb::base::SplitString(key, delim, &key_info);
        field->set_name(key_info.at(0));
        field->set_type(key_info.at(1));
        if (key_info.at(2) == "true") {
            field->set_is_key(true);
        } else {
            field->set_is_key(false);
        }
    }
    std::shared_ptr<Table> table_ptr = std::make_shared<Table>(table_name, schema);
    TableManifest* manifest = tablet_manifest_->add_table_manifest();
    manifest->CopyFrom(table_ptr->GetTableManifest());
    table_map_.insert(std::make_pair(table_name, table_ptr));
    table_manifest_map_.insert(std::make_pair(table_name, *manifest));
    return true;
}

// insert table_name key,key,key value,value,value
// TODO 判断输入参数是否正确
bool Tablet::Put(const ::ibdb::rpc::PutRequest* const request, ::ibdb::rpc::PutResponse* const response) {
    std::string statement = request->statement();
    std::string delim(" ");
    std::vector<std::string> result;
    ibdb::base::SplitString(statement, delim, &result);
    RpcCode code = RpcCode::OK;
    // 加锁，访问类变量
    std::lock_guard<std::mutex> lock(mu_);
    auto iterator = table_map_.find(result.at(1));
    if (iterator == table_map_.end()) {
        response->set_msg("table is not exist");
        response->set_code(RpcCode::FAILED);
        return false;
    }
    std::shared_ptr<Table> table = iterator->second;
    table->Put(statement);
    auto manifest = table_manifest_map_.find(result.at(1));
    if (manifest == table_manifest_map_.end()) {
        LOG(ERROR) << "no table manifest in map table_name[" << result.at(1) << "]";
        return false;
    }
    uint64_t offset = manifest->second.current_offset();
    offset++;
    manifest->second.set_current_offset(offset);
    response->set_msg("put ok");
    response->set_code(code);
    return true;
}

// get table_name key value timestamp
// TODO 判断输入参数是否正确
bool Tablet::Get(const ::ibdb::rpc::GetRequest* const request, ::ibdb::rpc::GetResponse* const response) {
    std::string statement = request->statement();
    std::string delim(" ");
    std::vector<std::string> result;
    ibdb::base::SplitString(statement, delim, &result);
    // 加锁，访问类变量
    std::lock_guard<std::mutex> lock(mu_);
    auto iterator = table_map_.find(result.at(1));
    RpcCode code = RpcCode::OK;
    if (iterator == table_map_.end()) {
        code = RpcCode::ERROR_NOT_FOUND;
        response->set_msg("table is not found");
        response->set_code(code);
        return false;
    }
    std::shared_ptr<Table> table = iterator->second;
    std::string message;
    table->Get(statement, message);
    response->set_msg(message);
    response->set_code(code);
    return true;
}

// if wrong return nullptr
// end_offset > start_offset
// TODO 设计的哲学，返回值的选择，指针，对象，对象引用，还是传值。如果出错能返回nullptr，这样合适吗，合理吗？怎样拥有合理的设计模式呢
std::vector<std::string> Tablet::GetStatements(const std::string &table_name, const uint64_t start_offset,
                                               const uint64_t end_offset) {
    auto it = table_map_.find(table_name);
    std::vector<std::string> data;
    if (it == table_map_.end()) {
        // return empty vector
        return data;
    }
    std::shared_ptr<Table> table = it->second;
    table->GetBatchData(start_offset, end_offset, data);
    return data;
}

} // tablet
} // ibdb



#endif // IBDB_TABLET_TABLET_H