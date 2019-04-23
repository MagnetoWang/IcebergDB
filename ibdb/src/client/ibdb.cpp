/*
 * @Author: MagnetoWang
 * @Date: 2019-04-15 19:53:20
 * @Last Modified by: MagnetoWang
 * @Last Modified time: 2019-04-15 20:11:28
 */
#include "client/tablet_client.h"

#include "glog/logging.h"
#include "gflags/gflags.h"

DECLARE_string(start_program);
DECLARE_string(ibdb_log_dir);

int main(int argc, char* argv[]) {
    ::google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    LOG(ERROR) << FLAGS_start_program;
    if (FLAGS_start_program == "StartNode") {

    }
    
}