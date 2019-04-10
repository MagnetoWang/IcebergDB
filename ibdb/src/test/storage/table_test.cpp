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

// TODO 这块逻辑不对啊，没有assert判断语句
TEST(TableTest, PutDisk) {
    std::string table_name("PutDisk");
    Schema schema;
    Field* name = schema.add_field();
    Field* id = schema.add_field();
    Field* gender = schema.add_field();
    Field* province = schema.add_field();
    Field* timestamp = schema.add_field();

    name->set_name("name");
    name->set_type("string");
    name->set_is_key(true);
    id->set_name("id");
    id->set_type("uint64_t");
    id->set_is_key(true);
    gender->set_name("gender");
    gender->set_type("string");
    gender->set_is_key(false);
    province->set_name("province");
    province->set_type("string");
    province->set_is_key(false);
    timestamp->set_name("timestamp");
    timestamp->set_type("uint64_t");
    timestamp->set_is_key(false);
    Table table(table_name, schema);

    std::string insert("insert PutDisk name magnetowang ");
    table.PutDisk(insert);
}

TEST(TableTest, PutIndex) {
    std::string table_name("PutIndex");
    Schema schema;
}

TEST(TableTest, PutAndGet) {
    std::string table_name("PutAndGet");
    Schema schema;
    Field* name = schema.add_field();
    Field* id = schema.add_field();
    Field* gender = schema.add_field();
    Field* province = schema.add_field();
    Field* timestamp = schema.add_field();

    name->set_name("name");
    name->set_type("string");
    name->set_is_key(true);
    id->set_name("id");
    id->set_type("uint64_t");
    id->set_is_key(true);
    gender->set_name("gender");
    gender->set_type("string");
    gender->set_is_key(false);
    province->set_name("province");
    province->set_type("string");
    province->set_is_key(false);
    timestamp->set_name("timestamp");
    timestamp->set_type("uint64_t");
    timestamp->set_is_key(false);
    Table table(table_name, schema);

    int number = 1000;
    std::string insert("insert");
    std::vector<std::string> insert_vec;
    insert_vec.reserve(number);
    insert = insert + " " + table_name + " name,id,gender,province,timestamp "; 
    for (int i = 0; i < number; i++) {
        std::string name_str = "name" + std::to_string(i);
        std::string id_str = "id" + std::to_string(i);
        std::string gender_str("male");
        if (i % 2 == 0) {
            gender_str = "female";
        }
        std::string province_str("beijing");
        std::string timestamp_str(std::to_string(i));
        std::string statement;
        statement = insert + name_str + "," + id_str + "," + gender_str + "," + province_str + "," + timestamp_str;
        // LOG(ERROR) << statement;
        // LOG(ERROR) << statement;
        bool result = table.Put(statement);
        ASSERT_EQ(result, true);
        insert_vec.push_back(statement);
    }
    std::string table_get("get");
    table_get = table_get + " " + table_name + " id ";
    for (int i = number -1 ; i >= 0; i--) {
        std::string id_str = "id" + std::to_string(i);
        std::string timestamp_str(std::to_string(i));
        std::string statement;
        statement = table_get + id_str + " " + timestamp_str;
        std::string message;
        bool result = table.Get(statement, message);
        ASSERT_EQ(result, true);
        ASSERT_EQ(insert_vec.at(i).size(), message.size());
        ASSERT_EQ(insert_vec.at(i), message);
    }
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