/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-14 21:50:38 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-05-06 10:40:40
 */
#include "tablet/tablet.h"

#include "protobuf/rpc.pb.h"
#include "protobuf/storage.pb.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::rpc::CreateRequest;
using ibdb::rpc::CreateResponse;
using ibdb::rpc::PutRequest;
using ibdb::rpc::PutResponse;
using ibdb::rpc::GetRequest;
using ibdb::rpc::GetResponse;

DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);

namespace ibdb {
namespace tablet {

class TabletTest {};

TEST(TabletTest, Simply) {
    std::string endpoint = "127.0.0.1:99991";
    bool is_leader = true;
    Tablet tablet(endpoint, is_leader);
    ASSERT_EQ(tablet.Init(), true);
}

TEST(TabletTest, CreateRequest) {
    std::string endpoint = "127.0.0.1:99991";
    bool is_leader = true;
    Tablet tablet(endpoint, is_leader);
    ASSERT_EQ(tablet.Init(), true);
    std::string table_name = "CreateRequest";
    std::string create_str = "create " + table_name + " name,string,true timestamp,uint64_t,false gender,bool,false id,uint64_t,true province,string,false";
    CreateRequest* create_request = new CreateRequest();
    create_request->set_statement(create_str);
    CreateResponse* create_reponse = new CreateResponse();
    tablet.Create(create_request, create_reponse);
}

TEST(TabletTest, PutAndGetRequest) {
    std::string endpoint = "127.0.0.1:99991";
    bool is_leader = true;
    Tablet tablet(endpoint, is_leader);
    // Remember tablet need to Init()
    ASSERT_EQ(tablet.Init(), true);
    std::string table_name = "PutAndGetRequest";
    std::string create_str = "create " + table_name + " name,string,true timestamp,uint64_t,false gender,bool,false id,uint64_t,true province,string,false";
    CreateRequest* create_request = new CreateRequest();
    create_request->set_statement(create_str);
    CreateResponse* create_reponse = new CreateResponse();
    tablet.Create(create_request, create_reponse);

    int number = 10;
    std::string insert = "insert " + table_name + " name,id,gender,province,timestamp ";
    std::vector<std::string> insert_vec;
    for (int i = 0; i < number; i++) {
        PutRequest* put_request = new PutRequest();
        PutResponse* put_reponse = new PutResponse();
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
        // LOG(ERROR) << statement;
        // LOG(ERROR) << statement;
        put_request->set_statement(statement);
        put_request->set_table_name(table_name);
        bool result = tablet.Put(put_request, put_reponse);
        ASSERT_EQ(result, true);
        ASSERT_EQ(put_reponse->code(), RpcCode::OK);
        insert_vec.push_back(statement);
    }

    std::string table_get("get");
    table_get = table_get + " " + table_name + " id ";
    for (int i = number -1 ; i >= 0; i--) {
        GetRequest* get_request = new GetRequest();
        GetResponse* get_response = new GetResponse();
        std::string id_str = "id" + std::to_string(i);
        std::string timestamp_str(std::to_string(i));
        std::string statement;
        statement = table_get + id_str + " " + timestamp_str;
        std::string message;
        get_request->set_statement(statement);
        get_request->set_table_name(table_name);
        bool result = tablet.Get(get_request, get_response);
        ASSERT_EQ(result, true);
        ASSERT_EQ(get_response->code(), RpcCode::OK);
        message = get_response->msg();
        ASSERT_EQ(insert_vec.at(i).size(), message.size());
        ASSERT_EQ(insert_vec.at(i), message);
    }
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
