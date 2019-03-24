#ifndef IBDB_STORAGE_TABLE_H
#define IBDB_STORAGE_TABLE_H
#include <atomic>
#include <mutex>

#include "base/noncopyable.h"
#include "base/skiplist.h"
#include "base/utils.h"
#include "disk.h"
#include "protobuf/storage.pb.h"

using ibdb::base::Noncopyable;
using ibdb::base::SkipList;

DECLARE_uint32(sparse_threshold);
DECLARE_string(db_root);

namespace ibdb {
namespace storage {

// Default parameters are legal
class Table : Noncopyable {

public:
    // Table();
    explicit Table(const std::string& table_name);
    explicit Table(const TableManifest& tmanifest);
    ~Table();
    bool Put(std::string& statement);
    bool Get(std::string& statement);
    bool Delete();
private:
    std::atomic<uint64_t> current_offset_;
    WritableFileHandle* wf_;
    RandomAccessFileHandle* rf_;
    TableManifest table_manifest_;
    std::map<std::string, std::map<uint64_t, uint32_t> > offset_pos_map_;
};

// for recover table
Table::Table(const TableManifest& tmanifest)
    :   current_offset_(tmanifest.current_offset()),
        table_manifest_(tmanifest) {
        wf_ = new WritableFileHandle(tmanifest.current_log_file().log_name(), tmanifest.current_log_file().current_pos());
        // TODO 读取日志索引文件，然后写入map中
        // for ()

}

// table initialization for the firt time
Table::Table(const std::string& table_name) : current_offset_(0) {
    std::string table_dir = FLAGS_db_root + "/" + table_name;
    std::string log_dir = table_dir + "/log";
    ibdb::base::MkdirRecur(log_dir);
    wf_ = new WritableFileHandle(table_dir);
}

bool Table::Put(std::string& statement) {
    Slice message(statement);
    uint32_t current_pos = wf_->GetCurrentPos();
    Status s = wf_->Append(current_offset_.load(std::memory_order_relaxed), message);
    if (!s.ok()) {
        return false;
    }
    current_offset_.fetch_add(1, std::memory_order_relaxed);
    if (current_offset_ % FLAGS_sparse_threshold == 0) {

    } else {
        
    }
    return false;
}

} // ibdb
} // storage

#endif //IBDB_STORAGE_TABLE_H
