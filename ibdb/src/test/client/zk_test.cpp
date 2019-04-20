/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-17 22:33:22 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-17 22:33:46
 */
#include "client/zk.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);
DECLARE_string(test_zk_servers);
DECLARE_uint32(session_timeout);
DECLARE_uint32(connection_timeout);
DECLARE_string(test_zk_root);

namespace ibdb {
namespace client {

class ZkTest {};

TEST(ZkTest, Simply) {
    std::string servers = FLAGS_test_zk_servers;
    uint32_t session_timeout = FLAGS_session_timeout;
    uint32_t connection_timeout = FLAGS_connection_timeout;
    ZkClient zkClient(servers, session_timeout, connection_timeout);
    ASSERT_TRUE(zkClient.Init());
    ASSERT_TRUE(zkClient.CloseZK());
}

TEST(ZkTest, CreateAndDeleteZKNode) {
    std::string servers = FLAGS_test_zk_servers;
    uint32_t session_timeout = FLAGS_session_timeout;
    uint32_t connection_timeout = FLAGS_connection_timeout;
    ZkClient zkClient(servers, session_timeout, connection_timeout);
    ASSERT_TRUE(zkClient.Init());
    std::string node = FLAGS_test_zk_root + "/create/node";
    std::string data = "FLAGS_test_zk_root/create/node/data";
    bool result = zkClient.CreateNode(node, data, 0, 0, true);
    ASSERT_TRUE(result);
    // do not duplicate the same node
    result = zkClient.CreateNode(node, data, 0, 0, true);
    ASSERT_FALSE(result);
    result = zkClient.DeleteNode(node);
    ASSERT_TRUE(result);
    ASSERT_TRUE(zkClient.CloseZK());
}

TEST(ZkTest, GetChildren) {
    std::string servers = FLAGS_test_zk_servers;
    uint32_t session_timeout = FLAGS_session_timeout;
    uint32_t connection_timeout = FLAGS_connection_timeout;
    ZkClient zkClient(servers, session_timeout, connection_timeout);
    ASSERT_TRUE(zkClient.Init());
    int number = 10;
    std::string path = FLAGS_test_zk_root + "/getChildren";
    for (int i = 0; i < number; i++) {
        std::string node =  path + "/127.0.0.1:" + std::to_string(i);
        std::string data = "127.0.0.1:"  + std::to_string(i);
        bool result = zkClient.CreateNode(node, data, 0, 0, true);
        ASSERT_TRUE(result);
        result = zkClient.CreateNode(node, data, 0, 0, true);
        ASSERT_FALSE(result);
    }

    std::vector<std::string> children;
    zkClient.GetChildren(path, children);
    for (int i = 0; i < number; i++) {
        std::string node =  path + "/127.0.0.1:" + std::to_string(i);
        std::string data = "127.0.0.1:"  + std::to_string(i);
        ASSERT_EQ(children.at(i), data);
        bool result = false;
        result = zkClient.DeleteNode(node);
        ASSERT_TRUE(result);
    }
    ASSERT_TRUE(zkClient.CloseZK());
}


TEST(ZkTest, SetAndGetData) {
    std::string servers = FLAGS_test_zk_servers;
    uint32_t session_timeout = FLAGS_session_timeout;
    uint32_t connection_timeout = FLAGS_connection_timeout;
    ZkClient zkClient(servers, session_timeout, connection_timeout);
    ASSERT_TRUE(zkClient.Init());
    int number = 10;
    const std::string path = FLAGS_test_zk_root + "/SetAndGetData";
    zkClient.DeleteAllOfNodes(path);
    for (int i = 0; i < number; i++) {
        std::string node =  path + "/127.0.0.1:" + std::to_string(i);
        std::string data = "127.0.0.1:"  + std::to_string(i);
        bool result = zkClient.CreateNode(node, data, 0, 0, true);
        ASSERT_TRUE(result);
        std::string node_data;
        result = zkClient.GetData(node, node_data);
        ASSERT_TRUE(result);
        ASSERT_EQ(node_data, data);
        result = zkClient.CreateNode(node, data, 0, 0, true);
        ASSERT_FALSE(result);
    }

    for (int i = 0; i < number; i++) {
        std::string node =  path + "/127.0.0.1:" + std::to_string(i);
        std::string data = std::to_string(i) + "127.0.0.1:";
        bool result = zkClient.CreateNode(node, data, 0, 0, true);
        ASSERT_FALSE(result);
        result = zkClient.SetData(node, data);
        ASSERT_TRUE(result);

        std::string node_data;
        result = zkClient.GetData(node, node_data);
        ASSERT_TRUE(result);
        ASSERT_EQ(node_data, data);
        result = zkClient.CreateNode(node, data, 0, 0, true);
        ASSERT_FALSE(result);
    }
    bool result = zkClient.DeleteAllOfNodes(path);
    ASSERT_TRUE(result);
    ASSERT_TRUE(zkClient.CloseZK());

}

TEST(ZkTest, CheckNodeExisted) {

}

} // client
} // ibdb

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}