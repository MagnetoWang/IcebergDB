/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-25 16:31:27 
 * @Last Modified by:   MagnetoWang 
 * @Last Modified time: 2019-04-25 16:31:27 
 */

#include "cluster/tablet_node.h"

#include "protobuf/rpc.pb.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

DECLARE_string(ibdb_log_dir);
DECLARE_string(test_zk_servers);

using ibdb::rpc::PutRequest;
using ibdb::rpc::PutResponse;
using ibdb::rpc::GetRequest;
using ibdb::rpc::GetResponse;
//using ibdb::rpc::CreateRequest;
//using ibdb::rpc::CreateResponse;
using ibdb::rpc::TabletService_Stub;

namespace ibdb {
namespace cluster {

class TabletNodeTest {};

TEST(TabletNodeTest, Simply) {
    std::string endpoint("127.0.0.1:1990");
    std::string zk_endpoint("127.0.0.1:2181");
    TabletNode* node = new TabletNode(endpoint, zk_endpoint);
    ASSERT_TRUE(node->Init());
    delete node;

    // delete zk data
    std::string servers = FLAGS_test_zk_servers;
    uint32_t session_timeout = FLAGS_session_timeout;
    uint32_t connection_timeout = FLAGS_connection_timeout;
    ZkClient zkClient(servers, session_timeout, connection_timeout);
    ASSERT_TRUE(zkClient.Init());
    zkClient.DeleteNode(FLAGS_zk_select_node);
    zkClient.DeleteAllOfNodes(FLAGS_zk_node_list + "/127.0.0.1:1990");
    zkClient.DeleteAllOfNodes(FLAGS_zk_node_list);

//    ASSERT_TRUE(zkClient.CloseZK());
}


TEST(TabletNodeTest, StartTwoNodes) {
    std::string endpoint("127.0.0.1:1990");
    std::string zk_endpoint("127.0.0.1:2181");
    TabletNode* node1 = new TabletNode(endpoint, zk_endpoint);
    ASSERT_TRUE(node1->Init());
    delete node1;
    endpoint = "127.0.0.1:1991";
    zk_endpoint = "127.0.0.1:2181";
    TabletNode* node2 = new TabletNode(endpoint, zk_endpoint);
    ASSERT_TRUE(node2->Init());
    delete node2;

    // delete zk data
    std::string servers = FLAGS_test_zk_servers;
    uint32_t session_timeout = FLAGS_session_timeout;
    uint32_t connection_timeout = FLAGS_connection_timeout;
    ZkClient zkClient(servers, session_timeout, connection_timeout);
    ASSERT_TRUE(zkClient.Init());
    zkClient.DeleteNode(FLAGS_zk_select_node);
    zkClient.DeleteAllOfNodes(FLAGS_zk_node_list + "/127.0.0.1:1990");
    zkClient.DeleteAllOfNodes(FLAGS_zk_node_list + "/127.0.0.1:1991");
    // TODO 有bug
//    ASSERT_TRUE(zkClient.CloseZK());
}

TEST(TabletNodeTest, CreateTableRpc) {
    std::string endpoint("127.0.0.1:1990");
    std::string zk_endpoint("127.0.0.1:2181");
    TabletNode* node1 = new TabletNode(endpoint, zk_endpoint);
    ASSERT_TRUE(node1->Init());
    endpoint = "127.0.0.1:1991";
    zk_endpoint = "127.0.0.1:2181";
    TabletNode* node2 = new TabletNode(endpoint, zk_endpoint);
    ASSERT_TRUE(node2->Init());
    std::string servers = FLAGS_test_zk_servers;
    uint32_t session_timeout = FLAGS_session_timeout;
    uint32_t connection_timeout = FLAGS_connection_timeout;
    ZkClient zkClient(servers, session_timeout, connection_timeout);
    ASSERT_TRUE(zkClient.Init());

    std::string table_name("CreateTableRpc");
    ibdb::rpc::CreateRequest create_request;
    ibdb::rpc::CreateResponse create_response;
    std::string create_str = "create " + table_name + " name,string,true timestamp,uint64_t,false gender,bool,false id,uint64_t,true province,string,false";
    create_request.set_statement(create_str);

    std::string leader_endpoint;
    zkClient.GetData(FLAGS_zk_select_node, leader_endpoint);

    // start server
//    if (node1->endpoint() == leader_endpoint) {
//        node1->StartSever();
//    }
//    if (node2->endpoint() == leader_endpoint) {
//        node2->StartSever();
//    }
    RpcClient<TabletService_Stub>* client = new RpcClient<TabletService_Stub>(leader_endpoint);
    ASSERT_TRUE(client->Init());
    client->SendRequest(&TabletService_Stub::Create, &create_request, &create_response, FLAGS_timeout_ms, FLAGS_max_retry);
    LOG(INFO) << create_response.msg();
    delete node1;
    delete node2;

    zkClient.DeleteNode(FLAGS_zk_select_node);
    zkClient.DeleteAllOfNodes(FLAGS_zk_node_list + "/127.0.0.1:1990");
    zkClient.DeleteAllOfNodes(FLAGS_zk_node_list + "/127.0.0.1:1991");
}


} // cluster
} // ibdb

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}