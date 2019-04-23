#ifndef IBDB_CLIENT_TABLET_CLIENT_H
#define IBDB_CLIENT_TABLET_CLIENT_H

#include <mutex>

#include "rpc.h"
#include "zk.h"
#include "tablet/tablet.h"
#include "protobuf/rpc.pb.h"

#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::rpc::TabletService;
using ibdb::rpc::TabletService_Stub;
using ibdb::client::RpcClient;
using ibdb::client::ZkClient;

namespace ibdb {
namespace client {

// send request to any tablet node which you dont care for
class TabletClient {
public:
    TabletClient(const std::string& nodes, const std::string& );
    ~TabletClient();
    bool Init();
    bool Create(const std::string& statement);
    bool Put(const std::string& statement);
    bool Get(const std::string& statement);
    bool GetLeaderEndpoint();

private:
    std::mutex mu_;
    // tablet client's endpoint
    std::string endpoint_;
    // current tablet node's endpoint
    std::string current_endpoint_;
    // all of tablet node's endpoints
    std::vector<std::string> endpoints_;
    // zk root path
    std::string zk_root_path_;
    // send request by rpc_client_
    RpcClient<TabletService_Stub>* rpc_client_;
    // select leader by zk_client_
    ZkClient* zk_client_;
};

//TabletClient::TabletClient(const ) {
//
//}


} // client
} // ibdb

#endif // IBDB_CLIENT_TABLET_CLIENT_H