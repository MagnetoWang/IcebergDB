/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-16 15:19:06 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-16 20:04:49
 */


#include "protobuf/rpc.pb.h"
#include "cluster/tablet_node.h"
#include "port/port.h"

#include "brpc/server.h"
#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::rpc::CreateRequest;
using ibdb::rpc::CreateResponse;
using ibdb::client::RpcClient;
using ibdb::rpc::TabletService;
using ibdb::rpc::TabletService_Stub;
using ibdb::cluster::TabletNode;

DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);
DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    brpc::Server server;
    std::string endpoint = "127.0.0.1:55536";
    bool is_leader = true;
    TabletNode node(endpoint, is_leader);
    int result = server.AddService(&node, brpc::SERVER_DOESNT_OWN_SERVICE);
    if (result != 0 ) {
        LOG(ERROR) << "Fail to add node";
        return -1;
    }
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_timeout_ms;
    if (server.Start(55530, &options) != 0) {
        LOG(ERROR) << "Fail to start rpc server";
        return -1;
    }
    server.RunUntilAskedToQuit();
    return 0;
}