/*
 * @Author: MagnetoWang
 * @Date: 2019-04-15 19:53:20
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-15 20:11:28
 */
#include "client/tablet_client.h"
#include "cluster/tablet_node.h"

#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::cluster::TabletNode;

DECLARE_string(start_program);
DECLARE_string(ibdb_log_dir);
DECLARE_string(tablet_endpoint);
DECLARE_string(start_zk_endpoint);
DECLARE_string(start_endpoint);
DECLARE_string(test_zk_servers);

int main(int argc, char* argv[]) {
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    LOG(ERROR) << FLAGS_start_program;
    if (FLAGS_start_program == "StartNode") {
        TabletNode* node = new TabletNode(FLAGS_start_endpoint, FLAGS_start_zk_endpoint);
        node->StartSever(FLAGS_start_endpoint);
//        if (node->Init()) {
//
//        }
    }
    if (FLAGS_start_program == "StartClient") {
        TabletNode* node = new TabletNode(FLAGS_start_endpoint, FLAGS_start_zk_endpoint);
        if (node->Init()) {
            std::string servers = FLAGS_test_zk_servers;
            uint32_t session_timeout = FLAGS_session_timeout;
            uint32_t connection_timeout = FLAGS_connection_timeout;
            ZkClient zkClient(servers, session_timeout, connection_timeout);
//            ASSERT_TRUE();
            zkClient.Init();
            std::string table_name("CreateTableRpc");
            ibdb::rpc::CreateRequest create_request;
            ibdb::rpc::CreateResponse create_response;
            std::string create_str = "create " + table_name + " name,string,true timestamp,uint64_t,false gender,bool,false id,uint64_t,true province,string,false";
            create_request.set_statement(create_str);

            std::string leader_endpoint;
            zkClient.GetData(FLAGS_zk_select_node, leader_endpoint);
            RpcClient<TabletService_Stub>* client = new RpcClient<TabletService_Stub>(leader_endpoint);
//            ASSERT_TRUE();
            client->Init();
            client->SendRequest(&TabletService_Stub::Create, &create_request, &create_response, FLAGS_timeout_ms, FLAGS_max_retry);
            LOG(INFO) << create_response.msg();
        }
    }
}

// run it in bash
// ./ibdbs --start_program="StartNode" --start_endpoint="127.0.0.1:1990" --start_zk_endpoint="127.0.0.1:2181"
// ./ibdbs --start_program="StartNode" --start_endpoint="127.0.0.1:1991" --start_zk_endpoint="127.0.0.1:2181"


// ./ibdbs --start_program="StartClient" --start_endpoint="127.0.0.1:1991" --start_zk_endpoint="127.0.0.1:2181"