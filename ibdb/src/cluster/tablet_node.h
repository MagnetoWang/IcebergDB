
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
#include "brpc/server.h"

using ibdb::base::Noncopyable;
using ibdb::tablet::Tablet;
using ibdb::client::RpcClient;
using ibdb::rpc::TabletService;
using ibdb::port::RpcCode;

namespace ibdb {
namespace cluster {

class TabletNode : public TabletService {

public:
    TabletNode(std::string& endpoint, bool is_leader);
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
    std::mutex mu_;
    std::string endpoint_;
    std::atomic<bool> is_leader_;
    std::shared_ptr<Tablet> tablet_;
    // std::map<std::string, std::shared_ptr<Tablet>> table_tablet_;
    // RpcClient<TabletService> client_;
};

TabletNode::TabletNode(std::string& endpoint, bool is_leader) : endpoint_(endpoint), is_leader_(is_leader) {
    tablet_ = std::make_shared<Tablet>(endpoint_, is_leader_);
    tablet_->Init();
}

// TODO 智能指针析构问题
TabletNode::~TabletNode() {

}

bool TabletNode::Init() {
    return true;
}

void TabletNode::Create(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::CreateRequest* request,
                    ::ibdb::rpc::CreateResponse* response,
                    ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);

    if (!tablet_->Create(request)) {
        response->set_msg("failed to create data[" + request->statement() + "]");
        response->set_code(RpcCode::FAILED);
        LOG(ERROR) << response->msg();
        return;
    }
    response->set_msg("create table ok");
    response->set_code(RpcCode::OK);
    // return;
    // done->Run();
}

void TabletNode::Put(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::PutRequest* request,
                    ::ibdb::rpc::PutResponse* response,
                    ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    if (!tablet_->Put(request)) {
        LOG(ERROR) << "failed to put data[" << request->statement() << "]";
        response->set_code(RpcCode::FAILED);
        return;
    }
    response->set_msg("put data ok");
    response->set_code(RpcCode::OK);
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