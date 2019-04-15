/*
 * @Author: MagnetoWang
 * @Date: 2019-04-14 15:37:14
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-14 16:27:39
 */

#include "client/rpc.h"

#include "protobuf/rpc.pb.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);
namespace ibdb {
namespace client {

class RpcClientTest {};

Test(RpcClientTest, Simply) {
    
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

// gtest-adapter.debugConfig