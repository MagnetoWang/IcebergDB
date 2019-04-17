/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-12 22:40:44 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-17 12:34:59
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

namespace ibdb {
namespace cluster {

class TabletNode : public TabletService {

public:
    TabletNode(std::string& endpoint, bool is_leader);
    ~TabletNode();
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
    // RpcClient<TabletService> client_;
};

TabletNode::TabletNode(std::string& endpoint, bool is_leader) : endpoint_(endpoint), is_leader_(is_leader) {
    tablet_ = std::make_shared<Tablet>(endpoint_, is_leader_);
}

// TODO 智能指针析构问题
TabletNode::~TabletNode() {

}

void TabletNode::Create(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::CreateRequest* request,
                    ::ibdb::rpc::CreateResponse* response,
                    ::google::protobuf::Closure* done) {
    brpc::ClosureGuard done_guard(done);
    response->set_msg(request->statement());
    response->set_code(0);
    // done->Run();
}

void TabletNode::Put(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::PutRequest* request,
                    ::ibdb::rpc::PutResponse* response,
                    ::google::protobuf::Closure* done) {
    // tablet_->
    done->Run();
    return;
}
                    
void TabletNode::Get(::google::protobuf::RpcController* controller,
                    const ::ibdb::rpc::GetRequest* request,
                    ::ibdb::rpc::GetResponse* response,
                    ::google::protobuf::Closure* done) {
    done->Run();
    return;

}

} // cluster
} // ibdb



#endif // IBDB_CLUSTER_TABLET_NODE_H