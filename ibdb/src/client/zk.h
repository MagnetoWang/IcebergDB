#ifndef IBDB_CLIENT_ZK_H
#define IBDB_CLIENT_ZK_H

#include <mutex>

#include "base/utils.h"

#include "zookeeper/zookeeper.h"
#include "boost/function.hpp"
#include "glog/logging.h"
#include "gflags/gflags.h"

namespace ibdb {
namespace client {

// TODO 后面有时间开发

template <typename T>
class RetryPolicy {
public:
    RetryPolicy() {};
private:
    std::string policy_;
};

typedef boost::function<bool(std::vector<std::string>& endpoints)> Callback;

// TODO 需要提供参数验证的功能,面对不合法的参数,返回错误才行
class ZkClient {
public:
    ZkClient(const std::string& servers, uint32_t session_timeout, uint32_t connection_timeout);
    ~ZkClient();
    bool Init();
    bool CreateNode(const std::string& path, const std::string& data, uint32_t mode, uint32_t acl, bool create_parent = false);
    bool CreateParent(const std::string& path);
    bool DeleteNode(const std::string& path);
    bool GetChildren(const std::string& path, std::vector<std::string>& children);
    bool GetData(const std::string& path, std::string& data);
    bool SetData(const std::string& path, const std::string& data);
    bool CheckNodeExisted(const std::string& path);
    bool DeleteAllOfNodes(const std::string& path);
//    static void WatcherFunction(zhandle_t *zh, int type, int state, const char *path,void *watcherCtxn

//    GetNode();
//    SetWatcher();
    bool ConnectZK();
    bool CloseZK();

    enum CreateMode { Ephemeral = 1,
                      SEQUENCE};

    enum ACLMode { OPEN_ACL_UNSAFE = 0,
                    READ_ACL_UNSAFE,
                    CREATOR_ALL_ACL};
private:
    std::mutex mu_;
    std::condition_variable cv_;
    std::string servers_;
    uint32_t  session_timeout_;
    uint32_t connection_timeout_;
    std::string client_path_;

    zhandle_t *zh_;
    bool connected_;

    std::map<uint32_t, struct ACL_vector> acl_map_;
};

// ZOOAPI implementation for signature of a watch function
void WatcherFunction(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx) {
//    if (state == ZOO_CONNECTED_STATE) {
//        connected_ = true;
//        cv_.notify_one();
//    }
};

ZkClient::ZkClient(const std::string& servers, uint32_t session_timeout, uint32_t connection_timeout)
    :   servers_(servers),
        session_timeout_(session_timeout),
        connection_timeout_(connection_timeout),
        zh_(nullptr),
        connected_(false) {}

ZkClient::~ZkClient() {
    if (zh_ != nullptr) {
        zookeeper_close(zh_);
    }
}

bool ZkClient::Init() {
    std::unique_lock<std::mutex> lock(mu_);
    zh_ = zookeeper_init(servers_.c_str(), WatcherFunction, session_timeout_, 0, 0, 0);
//    cv_.wait_for(lock, std::chrono::milliseconds(session_timeout_));
    acl_map_.insert(std::make_pair(0, ZOO_OPEN_ACL_UNSAFE));
    acl_map_.insert(std::make_pair(1, ZOO_READ_ACL_UNSAFE));
    acl_map_.insert(std::make_pair(2, ZOO_CREATOR_ALL_ACL));
    if (zh_ == nullptr) {
        LOG(ERROR) << "init zhander is failed";
        return false;
    }
    connected_ = true;
    return true;
}

/**
 *
 * @param path can't null or empty and need to valid. the format like this /zk/node/xxx
 * @param data can't null or empty
 * @param mode ZOO_EPHEMERAL = 1, ZOO_SEQUENCE = 2, normal = 0
 * @param acl OPEN_ACL_UNSAFE = 0, READ_ACL_UNSAFE = 1, CREATOR_ALL_ACL = 2
 * @param create_parent create parent if you need to do
 * @return
 */
bool ZkClient::CreateNode(const std::string &path, const std::string &data, uint32_t mode, uint32_t acl, bool create_parent) {
    if (zh_ == nullptr || !connected_) {
        LOG(ERROR) << "NOT CONNECTED";
        return false;
    }
    char* path_buffer = new char[path.size()];
    auto iterator = acl_map_.find(acl);
    if (iterator == acl_map_.end()) {
        LOG(ERROR) << "acl is invalid acl[" + std::to_string(acl) + "]";
        return false;
    }
    if (create_parent) {
        std::string parent = path.substr(0, path.find_last_of("/"));
        if(!CreateParent(parent)) {
            LOG(ERROR) << "create parent path happened to error path[" + path + "]";
            return false;
        }
    }
    std::lock_guard<std::mutex> lock(mu_);
    // ZOO_OPEN_ACL_UNSAFE is a completely open ACL
    int result = zoo_create(zh_, path.c_str(), data.c_str(), data.size(), &(iterator->second), mode, path_buffer, path.size());
    if (result == ZOO_ERRORS::ZOK) {
        return true;
    }
    LOG(ERROR) << "create node happened to error[" + std::to_string(result) + "]";
    return false;
}

// Ephemeral nodes may not have children
// Creating parent node if this path need to do this.
bool ZkClient::CreateParent(const std::string &parent_path) {
    std::lock_guard<std::mutex> lock(mu_);
    std::string delim = "/";
    std::vector<std::string> nodes;
    ibdb::base::SplitString(parent_path, delim, &nodes);
    int result = false;
    std::string path = "";
    LOG(WARNING) << parent_path;
    for (std::string e : nodes) {
        if (e == "") {
            continue;
        }
        path += "/";
        path += e;
        LOG(WARNING) << path;
        char* path_buffer = new char[path.size()];
        // create normal node because ephemeral node don't have children
        result = zoo_create(zh_, path.c_str(), e.c_str(), e.size(), &ZOO_OPEN_ACL_UNSAFE, 0, path_buffer, path.size());
        // the same node is existed, so do not return false
        if (result == ZOO_ERRORS::ZNODEEXISTS) {
            continue;
        }
        if (result != ZOO_ERRORS::ZOK) {
            LOG(ERROR) << "create error[" + std::to_string(result) + "]";
            return false;
        }
    }
    return true;

}

bool ZkClient::DeleteNode(const std::string &path) {
    std::lock_guard<std::mutex> lock(mu_);
    if (zh_ == nullptr || !connected_) {
        return false;
    }
    int result = zoo_delete(zh_, path.c_str(), -1);
    if (result == ZOO_ERRORS::ZOK) {
        return true;
    }
    return false;
}

bool ZkClient::GetChildren(const std::string &path, std::vector<std::string> &children) {
    std::lock_guard<std::mutex> lock(mu_);
    if (zh_ == nullptr || !connected_) {
        return false;
    }
    // this struct is include int32_t count, char * *data
    struct String_vector strings;
    struct Stat stat;
    int result = zoo_get_children2(zh_, path.c_str(), 0, &strings, &stat);
    if (result == ZOO_ERRORS::ZOK) {
        for (int i = 0; i < strings.count; i++) {
            children.push_back(std::string(strings.data[i]));
        }
        std::sort(children.begin(), children.end());
        return true;
    }
    // TODO 所有函数都要添加错误日志
    LOG(ERROR) << "failed to get children path [" << path << "]";
    return false;
}

bool ZkClient::GetData(const std::string &path, std::string &data) {
    std::lock_guard<std::mutex> lock(mu_);
    if (zh_ == nullptr || !connected_) {
        return false;
    }
    char* buffer;
    int buffer_len = INT_MAX;
    struct Stat stat;
    int result = zoo_get(zh_, path.c_str(), 0, buffer, &buffer_len, &stat);
    if (result == ZOO_ERRORS::ZOK) {
        data.assign(buffer, buffer_len);
        return true;
    }
    return false;
}

bool ZkClient::SetData(const std::string &path, const std::string &data) {
    std::lock_guard<std::mutex> lock(mu_);
    if (zh_ == nullptr || !connected_) {
        return false;
    }
    struct Stat stat;
    int result = zoo_set2(zh_, path.c_str(), data.c_str(), data.size(), -1, &stat);
    if (result == ZOO_ERRORS::ZOK) {
        return true;
    }
    return false;
}

bool ZkClient::CheckNodeExisted(const std::string &path) {
    std::lock_guard<std::mutex> lock(mu_);
    if (zh_ == nullptr || !connected_) {
        return false;
    }
    struct Stat stat;
    int result = zoo_exists(zh_, path.c_str(), 0, &stat);
    if (result == ZOO_ERRORS::ZOK) {
        return true;
    }
    return false;
}

bool ZkClient::ConnectZK() {
    std::unique_lock<std::mutex> lock(mu_);
    zh_ = zookeeper_init(servers_.c_str(), WatcherFunction, session_timeout_, 0, 0, 0);
    if (zh_ == nullptr) {
        LOG(ERROR) << "connecting zk is failed";
        return false;
    }
    connected_ = true;
    return true;
}

bool ZkClient::CloseZK() {
    std::lock_guard<std::mutex> lock(mu_);
    if (zh_ == nullptr || !connected_) {
        return false;
    }
    int result = zookeeper_close(zh_);
    if (result == ZOO_ERRORS::ZOK) {
        connected_ = false;
        return true;
    }
    return false;
}

// do not add lock, or you will meet dead lock.
// getchildren and deleteNode functions had already have lock
bool ZkClient::DeleteAllOfNodes(const std::string &path) {
//    std::lock_guard<std::mutex> lock(mu_);
    if (zh_ == nullptr || !connected_) {
        return false;
    }
    std::vector<std::string> children;
    bool result = GetChildren(path, children);
    if (!result) {
        return false;
    }
    for (std::string e : children) {
        std::string node = path + "/" + e;
        DeleteNode(node);
    }
    return true;
}

} // client
} // ibdb


#endif // IBDB_CLIENT_ZK_H