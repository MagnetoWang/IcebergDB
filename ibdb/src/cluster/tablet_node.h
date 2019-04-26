/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-12 22:40:44 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-17 14:46:10
 */

#ifndef IBDB_CLUSTER_TABLET_NODE_H
#define IBDB_CLUSTER_TABLET_NODE_H
#include <mutex>
#include <limits.h>

#include "tablet/tablet.h"
#include "protobuf/rpc.pb.h"
#include "protobuf/storage.pb.h"
#include "client/rpc.h"
#include "client/zk.h"

#include "brpc/server.h"
#include "common/thread_pool.h"
#include "boost/bind.hpp"

using ibdb::base::Noncopyable;
using ibdb::tablet::Tablet;
using ibdb::client::RpcClient;
using ibdb::rpc::TabletService;
using ibdb::rpc::TabletService_Stub;
using ibdb::rpc::TabletManifest;
using ibdb::storage::TableManifest;
using ibdb::port::RpcCode;
using ibdb::client::ZkClient;
using ibdb::rpc::NodeInfomation;
using ibdb::rpc::BatchDataRequest;
using ibdb::rpc::BatchDataResponse;
using ibdb::rpc::GetRequest;
using ibdb::rpc::GetResponse;
using ibdb::rpc::PutRequest;
using ibdb::rpc::PutResponse;

DECLARE_string(zk_root);
DECLARE_string(zk_select_node);
DECLARE_string(zk_node_list);
DECLARE_uint32(session_timeout);
DECLARE_uint32(connection_timeout);
DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);
DECLARE_uint32(thread_interval);
DECLARE_uint32(max_thread_number);
DECLARE_uint32(sync_offset_threshold);

namespace ibdb {
namespace cluster {

// every node has own path in zookeeper and register own infomation in zookeeper by zk client
class TabletNode : public TabletService {

public:
    // for test function
    TabletNode(const std::string& endpoint, bool is_leader);
    //
    TabletNode(const std::string& endpoint, const std::string& zk_endpoint);
    ~TabletNode();
    bool Init();
    void Create(::google::protobuf::RpcController* controller,
                       const ::ibdb::rpc::CreateRequest* request,
                       ::ibdb::rpc::CreateResponse* response,
                       ::google::protobuf::Closure* done);
    void Put(::google::protobuf::RpcController* controller,
                       const ::ibdb::rpc::PutRequest* request,
                       ::ibdb::rpc::PutResponse* response,
                       ::google::protobuf::Closure* done);
    void Get(::google::protobuf::RpcController* controller,
                       const ::ibdb::rpc::GetRequest* request,
                       ::ibdb::rpc::GetResponse* response,
                       ::google::protobuf::Closure* done);
    void Delete(::google::protobuf::RpcController* controller,
                       const ::ibdb::rpc::DeleteRequest* request,
                       ::ibdb::rpc::DeleteResponse* response,
                       ::google::protobuf::Closure* done) {}
    void Update(::google::protobuf::RpcController* controller,
                       const ::ibdb::rpc::UpdateRequest* request,
                       ::ibdb::rpc::UpdateResponse* response,
                       ::google::protobuf::Closure* done) {}
   void GetTabletManifest(::google::protobuf::RpcController* controller,
                          const ::ibdb::rpc::GetTabletManifestRequest* request,
                          ::ibdb::rpc::GetTabletManifestResponse* response,
                          ::google::protobuf::Closure* done);
    void SendBatchData(::google::protobuf::RpcController* controller,
                       const ::ibdb::rpc::BatchDataRequest* request,
                       ::ibdb::rpc::BatchDataResponse* response,
                       ::google::protobuf::Closure* done);

private:
    bool RegisterZk();
    // return true means you are leader in current term, or you are not leader
    bool SelectLeader();
    void StartSever();
    // sync
    void SyncTabletData();
    bool SendTableData(const std::string& table_name, uint64_t offset, uint64_t next_node_offset);
    void UpdateTabletStatus();

private:
    std::mutex mu_;
    // for brpc server endpoint, not zk endpoint
    std::string endpoint_;
    std::atomic<bool> is_leader_;
    std::atomic<bool> is_alive_;
    std::shared_ptr<Tablet> tablet_;

    // node configuration
    std::string zk_endpoint_;
    ZkClient* zk_client_;
    std::string zk_root_path_;
    // leader send data to follower, however, follower cant do this.
    // follower send data to follower
    std::vector<std::string> nodes_list_;
    // for node communication
    RpcClient<TabletService_Stub>* rpc_client_;
    brpc::Server* server_;
    NodeInfomation* next_node_;

    // task dispatchers
    std::atomic<bool> running_;
    ::baidu::common::ThreadPool task_pool_;
};

TabletNode::TabletNode(const std::string& endpoint, bool is_leader) : endpoint_(endpoint), is_leader_(is_leader) {
    tablet_ = std::make_shared<Tablet>(endpoint_, is_leader_);
}

TabletNode::TabletNode(const std::string& endpoint, const std::string& zk_endpoint) : endpoint_(endpoint),
                                                is_leader_(false),
                                                is_alive_(false),
                                                tablet_(nullptr),
                                                zk_endpoint_(zk_endpoint),
                                                zk_client_(nullptr),
                                                zk_root_path_(FLAGS_zk_root),
                                                rpc_client_(nullptr),
                                                server_(nullptr),
                                                task_pool_(FLAGS_max_thread_number) {
    running_.store(false, std::memory_order_release);
    next_node_ = new NodeInfomation();
}

// TODO 智能指针析构问题
TabletNode::~TabletNode() {
    task_pool_.Stop(true);
    zk_client_->CloseZK();
    delete rpc_client_;
}

bool TabletNode::Init() {
    zk_client_ = new ZkClient(zk_endpoint_, FLAGS_session_timeout, FLAGS_connection_timeout);
    if (!zk_client_->Init()) {
        return false;
    }
    if (!RegisterZk()) {
        return false;
    }
    LOG(INFO) << endpoint_ << " register to zk successfully";
    if (!SelectLeader()) {
        LOG(INFO) << endpoint_ << " are not leader";
        tablet_ = std::make_shared<Tablet>(endpoint_, false);
    }
    tablet_ = std::make_shared<Tablet>(endpoint_, true);
    task_pool_.DelayTask(FLAGS_thread_interval, boost::bind(&TabletNode::UpdateTabletStatus, this));
    task_pool_.DelayTask(FLAGS_thread_interval, boost::bind(&TabletNode::SyncTabletData, this));
    return true;
}

// health has two status:alive or death
// status has two status:follower or leader
// current_offset is a number
bool TabletNode::RegisterZk() {
    std::string node_path = FLAGS_zk_node_list + "/" + endpoint_;
    if (zk_client_->IsExistedPath(node_path)) {
        return true;
    }
    int result = zk_client_->CreateNode(node_path, endpoint_, 0, 0, true);
    if (!result) {
        return false;
    }
    std::string attribute = node_path + "/health";
    result = zk_client_->CreateNode(attribute, "alive", 0, 0, false);
    if (!result) {
        return false;
    }
    attribute = node_path + "/status";
    result = zk_client_->CreateNode(attribute, "follower", 0, 0, false);
    if (!result) {
        return false;
    }
//    attribute = node_path + "/current_offset";
//    result = zk_client_->CreateNode(attribute, std::to_string(current_offset_), 0, 0, false);
//    if (!result) {
//        return false;
//    }
    return true;
}

bool TabletNode::SelectLeader() {
    std::string node_path = FLAGS_zk_select_node + "/leader";
    return zk_client_->CreateNode(node_path, endpoint_, 0, 0, true);
}

void TabletNode::UpdateTabletStatus() {
    std::string attribute;
    std::string path = FLAGS_zk_node_list;
    std::lock_guard<std::mutex> lock(mu_);
    bool result = zk_client_->GetChildren(FLAGS_zk_node_list, nodes_list_);
    if (!result) {
        LOG(ERROR) << "get children is wrong";
        task_pool_.DelayTask(FLAGS_thread_interval, boost::bind(&TabletNode::UpdateTabletStatus, this));
        return;
    }
    int after_node_flag = INT32_MAX;
    // find alive next node
    for (int i = 0; i < nodes_list_.size(); i++) {
        if (nodes_list_.at(i) == endpoint_) {
            after_node_flag = i;
        }
        if (after_node_flag < i) {
            int index = i;
            if (nodes_list_.size() == after_node_flag + 1) {
                index = 0;
            }
            next_node_->set_endpoint(nodes_list_.at(index));
            path = FLAGS_zk_node_list + "/" + next_node_->endpoint() + "/health";
            result = zk_client_->GetData(path, attribute);
            if (attribute != "alive") {
                continue;
            }
            if (result) {
                next_node_->set_health(attribute);
            }
            path = FLAGS_zk_node_list + "/" + next_node_->endpoint() + "/status";
            result = zk_client_->GetData(path, attribute);
            if (result) {
                next_node_->set_status(attribute);
            }
//            path = FLAGS_zk_node_list + "/" + next_node_ + "/current_offset";
//            result = zk_client_->GetData(path, attribute);
//            if (result == true) {
//                next_node_->set_current_offset(attribute);
//            }
            break;
        }
    }
    // init rpc client
    if (rpc_client_ == nullptr) {
        rpc_client_ = new RpcClient<TabletService_Stub>(next_node_->endpoint());
        rpc_client_->Init();
    } else {
        if (rpc_client_->endpoint() != next_node_->endpoint()) {
            delete rpc_client_;
            rpc_client_ = new RpcClient<TabletService_Stub>(next_node_->endpoint());
            rpc_client_->Init();
        }
    }
    task_pool_.DelayTask(FLAGS_thread_interval, boost::bind(&TabletNode::UpdateTabletStatus, this));
    return;
}

void TabletNode::SyncTabletData() {
    ibdb::rpc::GetTabletManifestRequest request;
    ibdb::rpc::GetTabletManifestResponse response;
    rpc_client_->SendRequest(&TabletService_Stub::GetTabletManifest, &request, &response, FLAGS_timeout_ms, FLAGS_max_retry);
    if (response.code() != RpcCode::OK) {
        LOG(ERROR) << "get tablet manifest failed code[" << std::to_string(response.code()) << "]";
    }
    ibdb::rpc::TabletManifest next_node_manifest(response.tablet_manifest());
    // only leader send data to follower or follow send data to follower
    if (!next_node_manifest.is_leader()) {
        for (int i = 0; i < next_node_manifest.table_manifest_size(); i++) {
            TableManifest next_table = next_node_manifest.table_manifest(i);
            TableManifest current_table = *(tablet_->GetTableManifest(next_table.name()));
            // TODO add sync task
            if (current_table.current_offset() > next_table.current_offset() + FLAGS_sync_offset_threshold) {
                task_pool_.AddTask(boost::bind(&TabletNode::SendTableData, this, next_table.name(), current_table.current_offset(), next_table.current_offset()));
            }
        }
    }
    task_pool_.DelayTask(FLAGS_thread_interval, boost::bind(&TabletNode::SyncTabletData, this));
}

void TabletNode::StartSever() {
    server_ = new brpc::Server();
    int result = server_->AddService(this, brpc::SERVER_DOESNT_OWN_SERVICE);
    if (result != 0) {
        LOG(ERROR) << "Fail to add Service";
        return;
    }
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_timeout_ms;
    if (server_->Start(endpoint_.c_str(), &options) != 0) {
        LOG(ERROR) << "Fail to start rpc server";
        return;
    }
    server_->RunUntilAskedToQuit();
    return;
}

void TabletNode::Create(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::CreateRequest* request,
                    ::ibdb::rpc::CreateResponse* response,
                    ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    if (!tablet_->Create(request, response)) {
        response->set_msg("failed to create data[" + request->statement() + "]");
        response->set_code(RpcCode::FAILED);
        LOG(ERROR) << response->msg();
        return;
    }
    return;
    // done->Run();
}

void TabletNode::Put(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::PutRequest* request,
                    ::ibdb::rpc::PutResponse* response,
                    ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    if (!tablet_->Put(request, response)) {
        LOG(ERROR) << "failed to put data[" << request->statement() << "]";
        response->set_code(RpcCode::FAILED);
        return;
    }
    return;
}

void TabletNode::Get(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::GetRequest* request,
                    ::ibdb::rpc::GetResponse* response,
                    ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    if (!tablet_->Get(request, response)) {
        LOG(ERROR) << "failed to get data[" << request->statement() << "]";
        response->set_code(RpcCode::FAILED);
        return;
    }
    return;
}

void TabletNode::GetTabletManifest(::google::protobuf::RpcController* controller,
                       const ::ibdb::rpc::GetTabletManifestRequest* request,
                       ::ibdb::rpc::GetTabletManifestResponse* response,
                       ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    std::string endpoint = request->endpoint();
    if (endpoint_ != endpoint) {
        LOG(ERROR) << "request endpoint is wrong";
        response->set_code(RpcCode::FAILED);
        response->set_msg("next node endpoint is not equal to request endpoint");
        return;
    }
    TabletManifest tablet_manifest(tablet_->GetTabletManifest());
    response->set_allocated_tablet_manifest(&tablet_manifest);
    response->set_msg("get manifest successfully");
    response->set_code(RpcCode::OK);
    return;
}

// actually we can believe that all of parameter is right and valid
// that is design of code
bool TabletNode::SendTableData(const std::string& table_name, uint64_t offset, uint64_t next_node_offset) {
    BatchDataRequest request;
    BatchDataResponse response;
    std::vector<std::string> data = tablet_->GetStatements(table_name, next_node_offset, offset);
    request.set_table_name(table_name);
    request.set_endpoint(next_node_->endpoint());
    for (auto e : data) {
        request.add_statement(e);
    }
    rpc_client_->SendRequest(&TabletService_Stub::SendBatchData, &request, &response, FLAGS_timeout_ms, FLAGS_max_retry);
}

void TabletNode::SendBatchData(::google::protobuf::RpcController *controller,
                               const ::ibdb::rpc::BatchDataRequest *request, ::ibdb::rpc::BatchDataResponse *response,
                               ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    for (int i = 0 ; i < request->statement_size(); i++) {
        std::string insert_statement = request->statement(i);
        PutRequest put_request;
        PutResponse put_response;
        put_request.set_table_name(request->table_name());
        put_request.set_statement(insert_statement);
        tablet_->Put(&put_request, &put_response);
        if (response->code() != RpcCode::OK) {
            LOG(ERROR) << "put failed";
            put_response.set_code(response->code());
            put_response.set_msg("put failed data[" + insert_statement + "]");
            return;
        }
    }
}

} // cluster
} // ibdb

#endif // IBDB_CLUSTER_TABLET_NODE_H