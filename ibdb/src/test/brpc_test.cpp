#include "brpc/butil/files/file.h"
#include "brpc/butil/files/file_path.h"
#include "base/utils.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
// TODO 解决brpc 和 glog之间的版本问题！！！
class BrpcTest {};
// DECLARE_string(db_root);
DECLARE_string(ibdb_log_dir);
TEST(BrpcTest, Simply){
    std::string test("test");
    ibdb::base::Mkdir(test);
    std::string paths("test/BrpcTest_Simply.txt");
    butil::FilePath path(paths);
    butil::File file(path, butil::File::FLAG_CREATE);
    ASSERT_EQ(0, file.GetLength());
    // LOG(INFO) << file.GetLength();
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}