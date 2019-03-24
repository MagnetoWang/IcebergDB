#include "storage/table.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
DECLARE_string(ibdb_log_dir);

namespace ibdb {
namespace storage {

class TableTest {};

TEST(TableTest, InitTable) {
    

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