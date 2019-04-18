/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-14 16:41:37 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-17 13:25:54
 */

#include "client/rpc.h"

#include "protobuf/rpc.pb.h"
#include "cluster/tablet_node.h"
#include "port/port.h"

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
    std::string endpoint = "127.0.0.1:55530";
    RpcClient<TabletService_Stub>* client = new RpcClient<TabletService_Stub>(endpoint);
    bool init = client->Init();
    assert(init == true);
    CreateRequest request;
    CreateResponse response;
    int number = 10;
    for (int i = 0; i < number; i++) {
        std::string send_string = "value" + std::to_string(i);
        request.set_statement(send_string);
        client->SendRequest(&TabletService_Stub::Create, &request, &response, FLAGS_timeout_ms, FLAGS_max_retry);
        if (response.msg() != request.statement()) {
            LOG(ERROR) << response.msg();
            LOG(ERROR) << response.code();
            LOG(ERROR) << request.statement();
        }
        assert(response.msg() == request.statement());
        LOG(INFO) << "send request successfully";
        ibdb::base::sleep(1000);
    }
}