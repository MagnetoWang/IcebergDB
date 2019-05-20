/*
 * @Author: MagnetoWang 
 * @Date: 2019-04-12 22:46:56 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-16 22:32:55
 */

#ifndef IBDB_CLIENT_RPC_H
#define IBDB_CLIENT_RPC_H
#include "base/noncopyable.h"
#include "port/port.h"

#include "boost/function.hpp"
#include "brpc/controller.h"
#include "brpc/channel.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::base::Noncopyable;
using ibdb::port::RpcCode;

DECLARE_int32(timeout_ms);
DECLARE_int32(max_retry);

namespace ibdb {
namespace client {

//typedef boost::function<void(::google::protobuf::RpcController* controller, const Request* Request, Response*, Callback*)>
// every client connect to single server node
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
    void SendRequest(void(Stub::*function)(::google::protobuf::RpcController* controller,
                     const Request*, Response*, Callback*),
                     const Request* request,
                     Response* response, uint32_t timeout_ms, uint32_t max_retry);
    std::string endpoint() {
        return endpoint_;
    }
private:
    // ip:port
    std::string endpoint_;
    uint64_t log_id_;
    brpc::Channel* channel_;
    brpc::ChannelOptions* channel_options_;
    Stub* service_stub_;
};

template<class Stub>
RpcClient<Stub>::~RpcClient() {
    delete channel_;
    delete channel_options_;
    delete service_stub_;
}

// build channel and init channel with endpoint once
template<class Stub>
bool RpcClient<Stub>::Init() {
    channel_ = new brpc::Channel();
    channel_options_ = new brpc::ChannelOptions();
    // channel_options_->protocol()
    channel_options_->timeout_ms = FLAGS_timeout_ms;
    channel_options_->max_retry = FLAGS_max_retry;
    if (channel_->Init(endpoint_.c_str(), channel_options_) != 0) {
        LOG(ERROR) << "Fail to initialize channel";
        return false;
    }
    service_stub_ = new Stub(channel_);
    return true;
}

template<class Stub>
template<class Request, class Response, class Callback>
void RpcClient<Stub>::SendRequest(void(Stub::*function)(::google::protobuf::RpcController* controller,
                            const Request*, Response*, Callback*),
                            const Request* request, Response* response,
                            uint32_t timeout_ms, uint32_t max_retry) {
    brpc::Controller cntl;
    uint64_t id = log_id_;
    LOG(ERROR) << id;
    cntl.set_log_id(id);
    cntl.set_timeout_ms(timeout_ms);
    cntl.set_max_retry(max_retry);
    (service_stub_->*function)(&cntl, request, response, nullptr);
    if (cntl.Failed()) {
//        LOG(ERROR) << "send request is wrong";
        LOG(ERROR) << cntl.ErrorText();
        response->set_msg("send request is wrong");
        response->set_code(RpcCode::FAILED);
    }
    log_id_++;
}

} // client
} // ibdb

#endif // IBDB_CLIENT_RPC_H