/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-12 22:46:56 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-15 14:23:11
 */

#ifndef IBDB_CLIENT_RPC_H
#define IBDB_CLIENT_RPC_H
#include "base/noncopyable.h"

#include "brpc/controller.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::base::Noncopyable;
DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);

namespace ibdb {
namespace client {

template<class Stub>
class RpcClient : Noncopyable {

public:
    RpcClient(const std::string endpoint) 
        :   endpoint_(endpoint),
            log_id_(0),
            channel_(nullptr),
            channel_options_(nullptr),
            service_stub_(nullptr) {};
    ~RpcClient();
    bool Init();
    template<class Request, class Response, class Callback>
    void SendRequest(void(T::*function)(::google::protobuf::RpcController* controller
                    const Request*, Response*, Callback*)
                        const Request* request,
                        Response* response, uint32_t timeout_ms, uint32_t max_retry);
private:
    // ip:port
    std::string endpoint_;
    uint64_t log_id_;
    brpc::Channel* channel_;
    brpc::ChannelOptions* channel_options_;
    Stub* service_stub_;
}

RpcClient::~RpcClient() {
    delete channel_;
    delete channel_options_;
    delete service_stub_;
}

bool RpcClient::Init() {
    channel_ = new brpc::Channel();
    channel_options_ = new brpc::ChannelOptions();
    // channel_options_->protocol()
    channel_options_->timeout_ms(FLAGS_timeout_ms);
    channel_options_->max_retry(FLAGS_max_retry);
    channel_->Init(endpoint_, channel_options_);
    service_stub_ = new Stub(channel_);
}

template<class Request, class Response, class Callback>
void RpcClient::SendRequest(void(T::*function)(::google::protobuf::RpcController* controller
                            const Request*, Response*, Callback*),
                            const Request* request, Response* response,
                            uint32_t timeout_ms, uint32_t max_retry) {
    brpc::Controller cntl;
    cntl.set_log_id(log_id_);
    cntl.set_timeout_ms(timeout_ms);
    cntl.set_max_retry(max_retry);
    (service_stub_->*function)(&cntl, request, response, nullptr);
    if (!cntl.Failed()) {
        LOG(ERROR) << "send request is wrong";
    }
}

} // client
} // ibdb

#endif // IBDB_CLIENT_RPC_H