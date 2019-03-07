#include "base/utils.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include <gflags/gflags.h>

#include <ctime>
class GlogTest {};


TEST(GlogTest, InfoLog) {
    LOG(INFO) << "this is glog logging INFO test " << std::endl;
    LOG(INFO) << "当前时间" << std::time(nullptr) << std::endl;
    LOG(ERROR) << "this is glog logging ERROR test " << std::endl;
    LOG(ERROR) << "当前时间" << std::time(nullptr) << std::endl;
    // LOG(FATAL) << "this is glog logging FATAL test " << std::endl;
    // LOG(FATAL) << "当前时间" << std::time(nullptr) << std::endl;
    LOG(WARNING) << "this is glog logging WARNING test " << std::endl;
    LOG(WARNING) << "当前时间" << std::time(nullptr) << std::endl;
}

int main(int argc, char **argv) {
    // ibdb::base::GlogInit();
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO,"/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/");  
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}