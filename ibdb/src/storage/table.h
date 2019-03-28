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
#include "disk.h"
#include "segment.h"
#include "protobuf/storage.pb.h"

using ibdb::base::Noncopyable;

DECLARE_uint32(sparse_threshold);
DECLARE_string(db_root);

namespace ibdb {
namespace storage {

// Default parameters are legal
// TODO table不能重复创建！！
class Table : Noncopyable {

public:
    // Table();
    Table(const std::string& table_name, const Schema& schema);
    explicit Table(const TableManifest& tmanifest);
    ~Table() {
        // if (wf_ != nullptr) {
        //     delete wf_;
        // }
        // if (rf_ != nullptr) {
        //     delete rf_;
        // }
    };
    bool PutDisk(std::string& statement);
    bool PutIndex(std::string& key, uint64_t timestamp, uint64_t offset);
    bool Get(std::string& statement);
    bool Delete();
    std::string GetTableManifestString();
private:

private:
    TableManifest table_manifest_;
    std::atomic<uint64_t> current_offset_;
    //Index
    // Segment* index_;
    // Disk
    WritableFileHandle* wf_;
    std::ofstream index_wf_;
    RandomAccessFileHandle* rf_;
    std::map<std::string, std::map<uint64_t, uint32_t>> offset_pos_map_;

    std::vector<LogIndexMap> log_index_;
};

// for recover table
Table::Table(const TableManifest& tmanifest)
    :   current_offset_(tmanifest.current_offset()),
        table_manifest_(tmanifest) {
}

// table initialization for the firt time
Table::Table(const std::string& table_name, const Schema& schema) : current_offset_(0) {
    std::string table_dir = FLAGS_db_root + "/" + table_name;
    std::string log_dir = table_dir + "/log";
    ibdb::base::MkdirRecur(log_dir);
    table_manifest_.set_name(table_name);
    table_manifest_.set_current_offset(0);

    Schema* schema_tmp = table_manifest_.mutable_schema();
    schema_tmp->CopyFrom(schema);

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
    std::string current_file = table_manifest_.current_log_file().index_name();
    // write message key to index file
    if (current_offset % FLAGS_sparse_threshold == 0) {
        std::map<std::string, std::map<uint64_t, uint32_t>>::iterator it = offset_pos_map_.find(current_file);
        if (it != offset_pos_map_.end()) {
            std::map<uint64_t, uint32_t> offset_pos = it->second;
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

bool PutIndex(std::string& key, uint64_t timestamp, uint64_t offset) {
    return true;

}

std::string Table::GetTableManifestString() {
    return table_manifest_.SerializeAsString();
}

} // ibdb
} // storage

#endif //IBDB_STORAGE_TABLE_H
