#include "storage/table.h"
#include <atomic>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>


#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);
namespace ibdb {
namespace storage {

class TableTest {};

TEST(TableTest, InitTable) {
    std::string table_name("student");
    Schema schema;
    Field field;
    field.set_name("name");
    field.set_type("string");
    field.set_is_key(true);
    Table table(table_name, schema);
    std::string path = FLAGS_db_root + "/" + table_name + "/log";
    std::string root("ibdb");
    // TODO 解决无法删除文件夹的问题
    LOG(INFO) << ibdb::base::RemoveFolder(path);
    // std::string table_meta_info = table.Ser
    // google::protobuf::TextFormat::PrintToString(*(&table), table_meta_info);
    // LOG(INFO) << table.GetTableManifestString();
}

TEST(TableTest, ReadManifest) {
    std::string table_name("student2");
    Schema schema;
    Field field;
    field.set_name("name");
    field.set_type("string");
    field.set_is_key(true);
    Table table(table_name, schema);
    // LOG(INFO) << table.GetTableManifestString();

}

TEST(TableTest, PutDisk) {
    std::string table_name("PutDisk");
    Schema schema;
    Field field;
    field.set_name("name");
    field.set_type("string");
    field.set_is_key(true);
    Table table(table_name, schema);
    std::string insert("insert PutDisk name magnetowang ");
    table.PutDisk(insert);
}


} // storage
} // ibdb

int main(int argc, char** argv) {
    // ibdb::base::GlogInit();
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}