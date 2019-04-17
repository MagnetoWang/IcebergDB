/*
 * @Author: MagnetoWang 
 * @Date: 2019-03-29 09:51:05 
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-16 15:57:22
 */
#ifndef IBDB_BASE_UTILS_H
#define IBDB_BASE_UTILS_H
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <chrono> // for timestamp
#include "port/port.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

DECLARE_string(log_dir_info);
DECLARE_string(log_dir_fatal);
DECLARE_string(log_dir_error);
DECLARE_string(log_dir_warn);

namespace ibdb {
namespace base {

inline bool GlogInit()
{
    google::InitGoogleLogging("");
    // #ifdef DEBUG_MODE
    //     google::SetStderrLogging(google::GLOG_INFO); //设置级别高于 google::INFO 的日志同时输出到屏幕
    // #else
    //     google::SetStderrLogging(google::GLOG_FATAL);//设置级别高于 google::FATAL 的日志同时输出到屏幕
    // #endif
    FLAGS_colorlogtostderr = true; //设置输出到屏幕的日志显示相应颜色
    // FLAGS_servitysinglelog = true;// 用来按照等级区分log文件

    google::SetLogDestination(google::GLOG_FATAL, FLAGS_log_dir_fatal.c_str()); // 设置 google::FATAL 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_ERROR, FLAGS_log_dir_error.c_str()); //设置 google::ERROR 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_WARNING, FLAGS_log_dir_warn.c_str()); //设置 google::WARNING 级别的日志存储路径和文件名前缀
    google::SetLogDestination(google::GLOG_INFO, FLAGS_log_dir_info.c_str()); //设置 google::INFO 级别的日志存储路径和文件名前缀
    FLAGS_logbufsecs = 0; //缓冲日志输出，默认为30秒，此处改为立即输出
    FLAGS_max_log_size = 100; //最大日志大小为 100MB
    FLAGS_stop_logging_if_full_disk = true; //当磁盘被写满时，停止日志输出
    //google::SetLogFilenameExtension("91_"); //设置文件名扩展，如平台？或其它需要区分的信息
    //google::InstallFailureSignalHandler(); //捕捉 core dumped (linux)
    //google::InstallFailureWriter(&Log); //默认捕捉 SIGSEGV 信号信息输出会输出到 stderr (linux)

    return true;
}

static inline void SplitString(const std::string& full,
                               const std::string& delim,
                               std::vector<std::string>* result) {
    result->clear();
    if (full.empty()) {
        return;
    }

    std::string tmp;
    std::string::size_type pos_begin = full.find_first_not_of(delim);
    std::string::size_type comma_pos = 0;

    while (pos_begin != std::string::npos) {
        comma_pos = full.find(delim, pos_begin);
        if (comma_pos != std::string::npos) {
            tmp = full.substr(pos_begin, comma_pos - pos_begin);
            pos_begin = comma_pos + delim.length();
        } else {
            tmp = full.substr(pos_begin);
            pos_begin = comma_pos;
        }

        if (!tmp.empty()) {
            result->push_back(tmp);
            tmp.clear();
        }
    }
}

static inline bool IsVisible(char c) {
    return (c >= 0x20 && c <= 0x7E);
}

// make folder in current path 
inline static bool Mkdir(const std::string& path) {
    const int dir_mode = 0777;
    int ret = ::mkdir(path.c_str(), dir_mode); 
    if (ret == 0 || errno == EEXIST) {
        return true; 
    }
    LOG(WARNING) << "mkdir is failed" << "path is " << path << "err[" << errno << " : " << strerror(errno) << "]";
    return false;
}

// inline static bool IsExists(const std::string& path) {
//     struct stat buf;
//     int ret = ::lstat(path.c_str(), &buf);
//     if (ret < 0) {
//         return false;
//     }
//     return true;
// }

// check buf is zero with size lenght
inline bool is_zero(const char* buff, const size_t size) {
    if (size >= sizeof(uint64_t)) {
        return (0 == *(uint64_t*)buff) &&
            (0 == memcmp(buff, buff + sizeof(uint64_t), size - sizeof(uint64_t)));
    } else if (size > 0) {
        return (0 == *(uint8_t*)buff) &&
            (0 == memcmp(buff, buff + sizeof(uint8_t), size - sizeof(uint8_t)));
    } else {
        return 0;
    }
}

// create multiple folders. Format is likely /tmp/db/log
static inline bool MkdirRecur(const std::string& dir_path) {
    size_t beg = 0;
    size_t seg = dir_path.find('/', beg);
    while (seg != std::string::npos) {
        if (seg + 1 >= dir_path.size()) {
            break; 
        }
        if (!Mkdir(dir_path.substr(0, seg + 1))) {
            return false; 
        }
        beg = seg + 1;
        seg = dir_path.find('/', beg);
    }
    return Mkdir(dir_path);
}

static inline bool RemoveFile(std::string& path) {
    if(::remove(path.c_str()) == 0) {
        return true;
    } else {
        return false;
    }
}

// a folder can be delete when it has no files
static inline bool RemoveFolder(std::string& path) {
    if(::rmdir(path.c_str()) == 0) {
        return true;
    } else {
        return false;
    }
}

static inline uint64_t GetMillisecondTimestamp() {
    auto now = std::chrono::system_clock::now();
    uint64_t ts = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    return ts;
}

static inline uint64_t GetMicrosecondsTimestamp() {
    auto now = std::chrono::system_clock::now();
    uint64_t ts = (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    return ts;
}

static inline uint64_t GetSecondTimestamp() {
    auto now = std::chrono::system_clock::now();
    uint64_t ts = (uint64_t)std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return ts;
}

// input microsecond
static inline void sleep(uint64_t micro_second) {
    usleep(micro_second);
}

} // base
} // ibdb

#endif // IBDB_BASE_UTILS_H
