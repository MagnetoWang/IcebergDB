/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-12 22:40:44 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-17 14:46:10
 */

#ifndef IBDB_CLUSTER_TABLET_NODE_H
#define IBDB_CLUSTER_TABLET_NODE_H
#include <mutex>

#include "tablet/tablet.h"
#include "protobuf/rpc.pb.h"
#include "client/rpc.h"
#include "client/zk.h"

#include "brpc/server.h"

using ibdb::base::Noncopyable;
using ibdb::tablet::Tablet;
using ibdb::client::RpcClient;
using ibdb::rpc::TabletService;
using ibdb::port::RpcCode;
using ibdb::client::ZkClient;

DECLARE_string(zk_root);
DECLARE_string(zk_select_node);
DECLARE_string(zk_node_list);
DECLARE_uint32(session_timeout);
DECLARE_uint32(connection_timeout);
DECLARE_int32(timeout_ms);

namespace ibdb {
namespace cluster {

// every node has own path in zookeeper and register own infomation in zookeeper by zk client
class TabletNode : public TabletService {

public:
    // for test function
    TabletNode(const std::string& endpoint, bool is_leader);
    //
    TabletNode(const std::string& endpoint);
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

private:
    bool RegisterZk();
    // return true means you are leader in current term, or you are not leader
    bool SelectLeader();
    void StartSever();
    bool SyncData();
    bool SendData(const std::string& endpoint);

private:
    std::mutex mu_;
    std::string endpoint_;
    std::atomic<bool> is_leader_;
    std::atomic<bool> is_alive_;
    std::atomic<uint64_t> current_offset_;
    std::shared_ptr<Tablet> tablet_;

    // node configuration
    ZkClient* zk_client_;
    std::string zk_root_path_;
    // leader send data to follower, however, follower cant do this.
    // follower send data to follower
    std::vector<std::string> nodes_list_;
    // for node communication
    RpcClient<TabletService>* rpc_client_;
    brpc::Server* server_;

    std::atomic<bool> running_;
};

TabletNode::TabletNode(const std::string& endpoint, bool is_leader) : endpoint_(endpoint), is_leader_(is_leader) {
    tablet_ = std::make_shared<Tablet>(endpoint_, is_leader_);
}

TabletNode::TabletNode(const std::string& endpoint) : endpoint_(endpoint),
                                                is_leader_(false),
                                                is_alive_(false),
                                                current_offset_(0),
                                                tablet_(nullptr),
                                                zk_client_(nullptr),
                                                zk_root_path_(FLAGS_zk_root),
                                                rpc_client_(nullptr),
                                                server_(nullptr) {}

// TODO 智能指针析构问题
TabletNode::~TabletNode() {

}

bool TabletNode::Init() {
    zk_client_ = new ZkClient(endpoint_, FLAGS_session_timeout, FLAGS_connection_timeout);
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
    return true;
}

bool TabletNode::RegisterZk() {
    std::string node_path = FLAGS_zk_node_list + "/" + endpoint_;
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
    attribute = node_path + "/current_offset";
    result = zk_client_->CreateNode(attribute, std::to_string(current_offset_), 0, 0, false);
    if (!result) {
        return false;
    }
    return true;
}

bool TabletNode::SelectLeader() {
    std::string node_path = FLAGS_zk_select_node + "/leader";
    return zk_client_->CreateNode(node_path, endpoint_, 0, 0, true);
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

} // cluster
} // ibdb

#endif // IBDB_CLUSTER_TABLET_NODE_H