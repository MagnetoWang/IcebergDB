#ifndef IBDB_STORAGE_TABLE_H
#define IBDB_STORAGE_TABLE_H
#include <atomic>
#include <mutex>
#include <fstream>
#include <map>
#include <vector>
#include <string>

#include "base/noncopyable.h"
#include "base/utils.h"
#include "base/cache.h"
#include "disk.h"
#include "segment.h"
#include "protobuf/storage.pb.h"

#include "boost/lexical_cast.hpp"
#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::base::Noncopyable;
using ibdb::base::LRUCacheMap;

DECLARE_uint32(log_index_sparse_threshold);
DECLARE_string(db_root);
DECLARE_string(table_timestamp);

namespace ibdb {
namespace storage {

// Default parameters are legal
// TODO table不能重复创建！！
class Table {

public:
    // Table();
    Table(const std::string& table_name, const Schema& schema);
    explicit Table(const TableManifest& tmanifest);
    ~Table() {};
    // TODO 添加const限定词
    bool PutDisk(std::string& statement);
    bool PutIndex(std::string& key, uint64_t& timestamp, std::string& value, uint64_t& offset);
    bool Put(std::string& statement);
    bool Get(std::string& statement, std::string& message);
    bool FindOffsetPosition(uint64_t& offset, uint64_t& start_offset, uint32_t& pos, std::string& file);
    bool FindMessage(uint64_t& offset, uint64_t& start_offset, uint32_t& pos, std::string& file, Slice* const result);
    bool Delete();
    std::string GetTableManifestString();
    TableManifest& GetTableManifest();

private:
    // store all of table's information
    TableManifest table_manifest_;
    // record global offset
    std::atomic<uint64_t> current_offset_;
    // for memory index
    Segment* segment_index_;
    // Disk
    // for reading and writing log files
    WritableFileHandle* wf_;
    RandomAccessFileHandle* rf_;
    // for index files
    std::ofstream index_wf_;
    // filename offset file_postion
    std::map<std::string, std::map<uint64_t, uint32_t>> offset_pos_map_;
    // TODO 后续做成cache结构，提高查询效率
    // std::map<uint64_t, std::string> offset_value_;
    LRUCacheMap<uint64_t, std::string> offset_message_;
    // it is useless now
    std::vector<LogIndexMap> log_index_;
};

// for recover table
Table::Table(const TableManifest& tmanifest)
    :   table_manifest_(tmanifest),
        current_offset_(tmanifest.current_offset()) {
}

// table initialization for the firt time
Table::Table(const std::string& table_name, const Schema& schema) : current_offset_(0) {
    std::string table_dir = FLAGS_db_root + "/" + table_name;
    std::string log_dir = table_dir + "/log";
    ibdb::base::MkdirRecur(log_dir);
    table_manifest_.set_name(table_name);
    table_manifest_.set_current_offset(0);
    // segment initialization
    segment_index_ = new Segment(4);
    Schema* schema_tmp = table_manifest_.mutable_schema();
    schema_tmp->CopyFrom(schema);
    // TODO timstamp正常来讲不应该是key，应该分成两个概念。一个内置的ts,一个是用户的ts。默认用户的ts不是key,而是直接内置到每个特征下的ts的索引中
    for (int i = 0; i < schema_tmp->field_size(); i++) {
        Field field = schema_tmp->field(i);
        if (field.is_key()) {
            Slice key(field.name());
            if (!segment_index_->BuildKeyIndex(key)) {
                LOG(ERROR) << "segment cant build index as key[" << key.data() << "]";
            }
        }
    }

    // disk initialization
    std::string log = log_dir + "/00000000.log";
    std::string index = log_dir + "/00000000.index";
    LogManifest* log_manifest = table_manifest_.add_log_manifest();
    log_manifest->set_log_name("00000000.log");
    log_manifest->set_current_pos(0);
    log_manifest->set_index_name("00000000.index");
    log_manifest->set_index_current_pos(0);

    table_manifest_.set_current_offset(0);
    LogManifest* current_log_file = table_manifest_.mutable_current_log_file();
    current_log_file->CopyFrom(*log_manifest);

    index_wf_.open(index.c_str());
    wf_ = new WritableFileHandle(log);

    std::ofstream table_stream(table_dir + "/table_manifest");
    table_stream << table_manifest_.SerializeAsString();
    table_stream.close();
}

//
bool Table::PutDisk(std::string& statement) {
    Slice message(statement);
    uint32_t current_pos = wf_->GetCurrentPos();
    uint64_t current_offset = current_offset_.load(std::memory_order_relaxed);
    // write message to disk
    Status s = wf_->Append(current_offset_.load(std::memory_order_relaxed), message);
    if (!s.ok()) {
        return false;
    }
    s = wf_->Sync();
    if (!s.ok()) {
        return false;
    }
    std::string current_file = table_manifest_.current_log_file().log_name();
    // write message current_offset and pos to index file
    if (current_offset % FLAGS_log_index_sparse_threshold == 0) {
        std::map<std::string, std::map<uint64_t, uint32_t>>::iterator it = offset_pos_map_.find(current_file);
        if (it != offset_pos_map_.end()) {
            std::map<uint64_t, uint32_t>& offset_pos = offset_pos_map_.at(current_file);
            // TODO 不能有重复或者更新current_offset_,这里要有if判断
            offset_pos.insert(std::make_pair(current_offset, current_pos));
        } else {
            std::map<uint64_t, uint32_t> offset_pos;
            offset_pos.insert(std::make_pair(current_offset, current_pos));
            offset_pos_map_.insert(std::make_pair(current_file, offset_pos));
        }
    }
    current_offset_.fetch_add(1, std::memory_order_relaxed);
    //update table manifest
    table_manifest_.set_current_offset(current_offset);
    return true;
}

// When key is existed
bool Table::PutIndex(std::string& key, uint64_t& timestamp, std::string& value, uint64_t& offset) {
    Slice key_slice(key);
    Slice value_slice(value);
    segment_index_->Put(key_slice, timestamp, value_slice, offset);
    return true;
}

std::string Table::GetTableManifestString() {
    return table_manifest_.SerializeAsString();
}

TableManifest& Table::GetTableManifest() {
    return table_manifest_;
}

// insert table_name key,key,key value,value,value
// assume the statement is legal
bool Table::Put(std::string& statement) {
    std::string delim(" ");
    std::vector<std::string> result;
    ibdb::base::SplitString(statement, delim, &result);
    uint64_t current_offset = current_offset_.load(std::memory_order_relaxed);
    if (!PutDisk(statement)){
        LOG(ERROR) << "table can't put data in disk data[" << statement << "]";
        return false;
    }
    delim = ",";
    std::vector<std::string> key_result;
    ibdb::base::SplitString(result.at(2), delim, &key_result);
    int pos = 0;
    for (auto e : key_result) {
        if (FLAGS_table_timestamp == e) {
            break;
        }
        pos++;
    }
    std::vector<std::string> value_result;
    ibdb::base::SplitString(result.at(3), delim, &value_result);
    uint64_t timestamp;
    try {
        timestamp = boost::lexical_cast<uint64_t>(value_result.at(pos));
    } catch (boost::bad_lexical_cast& ) {
        LOG(ERROR) << "string to timestamp is failed string[" << value_result.at(pos) << "]";
        return false;
    }
    for (int i = 0; i < key_result.size(); i++) {
        Slice key(key_result.at(i));
        if (segment_index_->Contains(key)) {
            if(!PutIndex(key_result.at(i), timestamp, value_result.at(i), current_offset)) {
                return false;
            }
        }
    }
    return true;
}

// create table_name ts_name,uint_64,isIndex col_name,type,isIndex  col_name,type,isIndex
// bool Table::Create(std::string& statement) {
//     std::string delim(" ");
//     std::vector<std::string> result;
//     ibdb::base::SplitString(statement, delim, &result);
    
// }

// get table_name key value timestamp
bool Table::Get(std::string& statement, std::string& message) {
    std::string delim(" ");
    std::vector<std::string> result;
    ibdb::base::SplitString(statement, delim, &result);
    // Get offset
    Slice key(result.at(2));
    Slice value(result.at(3));
    uint64_t timestamp = 0;
    try {
        timestamp = boost::lexical_cast<uint64_t>(result.at(4));
    } catch (boost::bad_lexical_cast& ) {
        LOG(ERROR) << "string to timestamp is failed";
    }
    uint64_t offset = 0;
    if(!segment_index_->Get(key, timestamp, value, offset)) {
        return false;
    }
    if (offset_message_.Contains(offset)) {
        LOG(INFO) << "offset is existed in cache";
        return true;
    }
    // find message by offset
    uint64_t start_offset = 0;
    uint32_t pos = 0;
    std::string file;
    if(!FindOffsetPosition(offset, start_offset, pos, file)) {
        return false;
    }
    // TODO 命名方式不好
    Slice* message_result = new Slice();
    if (!FindMessage(offset, start_offset, pos, file, message_result)) {
        return false;
    }
    std::string message_string(message_result->data());
    offset_message_.Insert(offset, message_string);
    message = message_string;
    return true;
}

// find file postion in offset_pos_map_
bool Table::FindOffsetPosition(uint64_t& offset, uint64_t& start_offset, uint32_t& pos, std::string& file) {
    // TODO 是否需要判断offset越界问题？
    std::map<std::string, std::map<uint64_t, uint32_t>>::iterator it = offset_pos_map_.begin();
    uint64_t log_offset = 0;
    uint32_t log_pos = 0;
    bool find_flags = false;
    for (; it != offset_pos_map_.end(); it++) {
        file = it->first;
        std::map<uint64_t, uint32_t> offset_map = it->second;
        // TODO 逻辑有问题
        for (auto it_map = offset_map.begin(); it_map != offset_map.end(); it_map++) {
            if (it_map->first > offset) {
                find_flags = true;
                break;
            } else {
                log_offset = it_map->first;
                log_pos = it_map->second;
            }
        }
        if (find_flags == true) {
            break;
        }
    }
    // TODO offset 有可能比目前的索引值都要大！
    // if (find_flags == false) {
    //     return false;
    // }
    pos = log_pos;
    start_offset = log_offset;
    return true;
}

// TODO 是否可以把这些函数抽象成一个工具方法，而不是单独类成员函数？
bool Table::FindMessage(uint64_t& offset, uint64_t& start_offset, uint32_t& pos, std::string& file, Slice* const result) {
    std::string log_file = FLAGS_db_root + "/" + table_manifest_.name() + "/log/" + file;
    rf_ = new RandomAccessFileHandle(log_file, start_offset);
    Slice* message = new Slice();
    while(true) {
        uint64_t message_offset = 0;
        Status s = rf_->GetMessageOffset(pos, message_offset);
        if (!s.ok()) {
            LOG(ERROR) << "get start_offset[" << start_offset << "] is failed";
            return false;
        }
        if (start_offset != message_offset) {
            LOG(ERROR) << start_offset;
            LOG(ERROR) << message_offset;
        }
        assert(start_offset == message_offset);
        if (offset == message_offset) {
            s = rf_->GetMessage(pos, message);
            if (!s.ok()) {
                LOG(ERROR) << "get message[" << message->data() << "] is failed";
                return false;
            }
            break;
        }
        // TODO 这块逻辑不确定是否可以正确跳出
        if (message_offset > offset) {
            break;
        }
        uint32_t message_size = 0;
        s = rf_->GetMessageSize(pos, message_size);
        if (!s.ok()) {
             LOG(ERROR) << "get message_size[" << message_size << "] is failed";
            return false;
        }
        pos = pos + 12 + message_size;
        start_offset++;
    }
    LOG(INFO) << "get message is successed message[" << message->data() << "]";
    *result = Slice(message->data(), message->size());
    return true;
}

} // ibdb
} // storage

#endif //IBDB_STORAGE_TABLE_H
