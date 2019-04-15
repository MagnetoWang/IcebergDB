/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-14 21:50:38 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-14 22:01:14
 */
#include "tablet/tablet.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);

namespace ibdb {
namespace tablet {

class TabletClass {};

TEST(TabletClass, Simply) {
    std::string endpoint = "127.0.0.1:99991";
    bool is_leader = true;
    Tablet tablet(endpoint, is_leader);
}

} // tablet
} // ibdb

int main(int argc, char** argv) {
    // ibdb::base::GlogInit();
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
