#include "protobuf/storage.pb.h"
#include "base/utils.h"

#include "google/protobuf/text_format.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
DECLARE_string(ibdb_log_dir);
namespace ibdb {
namespace storage {
class StorageProto {};

TEST(StorageProto, Simply) {
    TableManifest manifest;
    manifest.set_name("simply");
    Schema* schema = manifest.mutable_schema();
    schema->set_name("student_name");
    schema->set_type("string");
    schema->set_is_key(false);
    manifest.set_current_offset(10);
    manifest.set_current_log_file("00000002.log");
    manifest.set_current_index_file("000000002.index");
    manifest.add_index_name("000001.index");
    manifest.add_log_name("000001.log");
    LOG(INFO) << manifest.name();
    std::string out;
    LOG(INFO) << manifest.SerializeAsString();
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

// int main() {
//     ibdb::storage::LogEntry messages;
// }