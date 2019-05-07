/*
 * @Author: MagnetoWang 
 * @Date: 2019-05-06 10:39:59 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-05-06 13:51:07
 */
#include "storage/table.h"
#include "tablet/tablet.h"
#include "base/utils.h"

#include <atomic>
#include <numeric> 

#include "protobuf/rpc.pb.h"
#include "protobuf/storage.pb.h"
#include "client/rpc.h"
#include "cluster/tablet_node.h"

#include "brpc/server.h"
#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::client::RpcClient;
using ibdb::rpc::TabletService;
using ibdb::rpc::TabletService_Stub;
using ibdb::rpc::PutRequest;
using ibdb::rpc::PutResponse;
using ibdb::rpc::GetRequest;
using ibdb::rpc::GetResponse;
using ibdb::cluster::TabletNode;

DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);
DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);

namespace ibdb {
namespace storage {
class TablePerformance {};

// TEST(TablePerformance, Put) {
//     std::string table_name("PutPerformances");
//     Schema schema;
//     Field* name = schema.add_field();
//     Field* id = schema.add_field();
//     Field* gender = schema.add_field();
//     Field* province = schema.add_field();
//     Field* timestamp = schema.add_field();

//     name->set_name("name");
//     name->set_type("string");
//     name->set_is_key(true);
//     id->set_name("id");
//     id->set_type("uint64_t");
//     id->set_is_key(true);
//     gender->set_name("gender");
//     gender->set_type("string");
//     gender->set_is_key(false);
//     province->set_name("province");
//     province->set_type("string");
//     province->set_is_key(false);
//     timestamp->set_name("timestamp");
//     timestamp->set_type("uint64_t");
//     timestamp->set_is_key(false);
//     Table table(table_name, schema);

//     int number = 1000000;
//     std::string insert("insert");
//     std::vector<std::string> insert_vec;
//     insert_vec.reserve(number);
//     insert = insert + " " + table_name + " name,id,gender,province,timestamp "; 
//     std::vector<uint64_t> running_time;
//     for (int i = 0; i < number; i++) {
//         std::string name_str = "name" + std::to_string(i);
//         std::string id_str = "id" + std::to_string(i);
//         std::string gender_str("male");
//         if (i % 2 == 0) {
//             gender_str = "female";
//         }
//         std::string province_str("beijing");
//         std::string timestamp_str(std::to_string(i));
//         std::string statement;
//         statement = insert + name_str + "," + id_str + "," + gender_str + "," + province_str + "," + timestamp_str;
//         // LOG(ERROR) << statement;
//         // LOG(ERROR) << statement;
//         uint64_t start_timestamp = ibdb::base::GetMillisecondTimestamp();
//         bool result = table.Put(statement);
//         uint64_t end_timestamp = ibdb::base::GetMillisecondTimestamp();
//         running_time.push_back(end_timestamp - start_timestamp);
//         if (i % (number / 5) == 0) {
//             LOG(ERROR) << "put number is " << i;
//             LOG(ERROR) << "put the sum of running time is " << std::accumulate(running_time.begin() , running_time.end() , 0);
//         }
//         ASSERT_EQ(result, true);
//         insert_vec.push_back(statement);
//     }
    
//     uint64_t sum = std::accumulate(running_time.begin() , running_time.end() , 0);
//     double average = (double)sum/(double)running_time.size();
//     LOG(ERROR) << "put number is " << number;
//     LOG(ERROR) << "put the sum of running time is " << sum;
//     running_time.clear();
//     std::string table_get("get");
//     table_get = table_get + " " + table_name + " id ";
//     for (int i = number -1 ; i >= 0; i--) {
//         std::string id_str = "id" + std::to_string(i);
//         std::string timestamp_str(std::to_string(i));
//         std::string statement;
//         statement = table_get + id_str + " " + timestamp_str;
//         std::string message;

//         uint64_t start_timestamp = ibdb::base::GetMillisecondTimestamp();
//         bool result = table.Get(statement, message);
//         uint64_t end_timestamp = ibdb::base::GetMillisecondTimestamp();
//         running_time.push_back(end_timestamp - start_timestamp);
//         // LOG(ERROR) << i;
//         if (i % (number / 5) == 0) {
//             LOG(ERROR) << "get number is " << i;
//             LOG(ERROR) << "get the sum of running time is " << std::accumulate(running_time.begin() , running_time.end() , 0);
//         }
//         ASSERT_EQ(result, true);
//         ASSERT_EQ(insert_vec.at(i).size(), message.size());
//         ASSERT_EQ(insert_vec.at(i), message);
//     }
//     LOG(ERROR) << "get number is " << number;
//     LOG(ERROR) << "get the sum of running time is " << std::accumulate(running_time.begin() , running_time.end() , 0);
//     running_time.clear();
//     table_get = "get";
//     table_get = table_get + " " + table_name + " id ";
//     for (int i = number - 1 ; i >= 0; i--) {
//         std::string id_str = "id" + std::to_string(i);
//         std::string timestamp_str(std::to_string(i));
//         std::string statement;
//         statement = table_get + id_str + " " + timestamp_str;
//         std::string message;

//         uint64_t start_timestamp = ibdb::base::GetMillisecondTimestamp();
//         bool result = table.Get(statement, message);
//         uint64_t end_timestamp = ibdb::base::GetMillisecondTimestamp();
//         running_time.push_back(end_timestamp - start_timestamp);
//         if (i % (number / 5) == 0) {
//             LOG(ERROR) << "get number is " << i;
//             LOG(ERROR) << "get the sum of running time is " << std::accumulate(running_time.begin() , running_time.end() , 0);
//         }
//         ASSERT_EQ(result, true);
//         ASSERT_EQ(insert_vec.at(i).size(), message.size());
//         ASSERT_EQ(insert_vec.at(i), message);
//     }
//     LOG(ERROR) << "In cache get number is " << number;
//     LOG(ERROR) << "In cache get the sum of running time is " << std::accumulate(running_time.begin() , running_time.end() , 0);
// }
} // storage
} // ibdb

namespace ibdb {
namespace tablet {
    
} // tablet
} // ibdb

namespace ibdb {
namespace client {
class ClientPerformance {};

TEST(ClientPerformance, PutAndGet) {
    // start server
    // brpc::Server server;
    std::string endpoint = "127.0.0.1:55530";
    // bool is_leader = true;
    // TabletNode node(endpoint, is_leader);
    // int result = server.AddService(&node, brpc::SERVER_DOESNT_OWN_SERVICE);
    // if (result != 0 ) {
    //     LOG(ERROR) << "Fail to add node";
    //     // return -1;
    // }
    // brpc::ServerOptions options;
    // options.idle_timeout_sec = FLAGS_timeout_ms;
    // if (server.Start(55530, &options) != 0) {
    //     LOG(ERROR) << "Fail to start rpc server";
    //     // return -1;
    // }
    // LOG(ERROR) << "start server is ok";
    // start client
    // RpcClient<TabletService_Stub>* client = new RpcClient<TabletService_Stub>(endpoint);
    // bool init = client->Init();
    // assert(init == true);

    
    brpc::Channel channel_;
    brpc::ChannelOptions channel_options_;
    
    
    channel_options_.timeout_ms = FLAGS_timeout_ms;
    channel_options_.max_retry = FLAGS_max_retry;
    if (channel_.Init(endpoint.c_str(), &channel_options_) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        // return false;
        assert(channel_.Init(endpoint.c_str(), &channel_options_) == 0);
    }
    TabletService_Stub service_stub_(&channel_);
    brpc::Controller cntl;
    cntl.set_log_id(0);
    cntl.set_timeout_ms(FLAGS_timeout_ms);
    cntl.set_max_retry(FLAGS_max_retry);
    // create table
    std::string table_name = "RpcPerformance";
    std::string create_str = "create " + table_name + " name,string,true timestamp,uint64_t,false gender,bool,false id,uint64_t,true province,string,false";
    ::ibdb::rpc::CreateRequest create_request;// = new ibdb::rpc::CreateRequest();
    create_request.set_statement(create_str);
    // create_request.set_table_name()
    ::ibdb::rpc::CreateResponse create_reponse;// = new ibdb::rpc::CreateResponse();
    // client->SendRequest(&TabletService_Stub::Create, create_request, create_reponse, FLAGS_timeout_ms, FLAGS_max_retry);
    service_stub_.Create(&cntl, &create_request, &create_reponse, nullptr);
    // assert(create_reponse)
    // LOG(ERROR) << create_reponse.code();
    
    if (cntl.Failed()) {
         LOG(ERROR) << cntl.ErrorText();
    }
    assert(!cntl.Failed());
    // put data
    int number = 10;
    std::string insert("insert");
    std::vector<std::string> insert_vec;
    insert_vec.reserve(number);
    insert = insert + " " + table_name + " name,id,gender,province,timestamp "; 
    std::vector<uint64_t> running_time;
    for (int i = 0; i < number; i++) {
        std::string name_str = "name" + std::to_string(i);
        std::string id_str = "id" + std::to_string(i);
        std::string gender_str("male");
        if (i % 2 == 0) {
            gender_str = "female";
        }
        std::string province_str("beijing");
        std::string timestamp_str(std::to_string(i));
        std::string statement;
        statement = insert + name_str + "," + id_str + "," + gender_str + "," + province_str + "," + timestamp_str;
        PutRequest* put_request = new PutRequest();
        put_request->set_statement(statement);
        put_request->set_table_name(table_name);
        PutResponse* put_response = new PutResponse();
        uint64_t start_timestamp = ibdb::base::GetMillisecondTimestamp();
        // client->SendRequest(&TabletService_Stub::Put, put_request, put_response, FLAGS_timeout_ms, FLAGS_max_retry);
        uint64_t end_timestamp = ibdb::base::GetMillisecondTimestamp();
        running_time.push_back(end_timestamp - start_timestamp);
    }
    LOG(ERROR) << "get number is " << number;
    LOG(ERROR) << "get the sum of running time is " << std::accumulate(running_time.begin() , running_time.end() , 0);
    running_time.clear();
    // get data
}

} // client
} // ibdb

int main(int argc, char** argv) {
    // ibdb::base::GlogInit();
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}