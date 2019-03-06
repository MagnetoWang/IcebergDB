#ifndef IBDB_BASE_UTILS_H
#define IBDB_BASE_UTILS_H
#include "port/port.h"
#include "glog/logging.h"
#include <gflags/gflags.h>

DECLARE_string(log_dir_info);
DECLARE_string(log_dir_fatal);
DECLARE_string(log_dir_error);
DECLARE_string(log_dir_warn);

namespace ibdb {
namespace base {

bool GlogInit()
{
    google::InitGoogleLogging("");
    #ifdef DEBUG_MODE
        google::SetStderrLogging(google::GLOG_INFO); //设置级别高于 google::INFO 的日志同时输出到屏幕
    #else
        google::SetStderrLogging(google::GLOG_FATAL);//设置级别高于 google::FATAL 的日志同时输出到屏幕
    #endif
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

} // base
} // ibdb

#endif // IBDB_BASE_UTILS_H
