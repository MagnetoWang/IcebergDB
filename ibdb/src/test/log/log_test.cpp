#include "log/format.h"
#include "log/log_reader.h"
#include "log/log_writer.h"

#include "base/utils.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include <gflags/gflags.h>
using ibdb::log::WritableFile;
namespace ibdb {
namespace log {
class LogT {
public:
    LogT(std::string& writefile) : writefile_(writefile) {
        writestream_ = fopen(writefile_.c_str(), "w");
        dest_ = ibdb::log::NewWritableFile(writefile_, writestream_);
        writer_ = new IbdbWriter(dest_);
    }

    ~LogT() {
        if (writestream_ != nullptr) {
            fclose(writestream_);
        }
    }

// private:
    std::string writefile_;
    FILE* writestream_;
    WritableFile* dest_;
    IbdbWriter* writer_;
};

class LogTest {};


TEST(LogTest, AddRecord) {
    std::string filename("addrecordtest.txt");
    // FILE* writestream_ = fopen(filename.c_str(), "w");
    // WritableFile* dest = ibdb::log::NewWritableFile(filename, writestream_);
    // Writer* writer = new Writer(dest);
    // ibdb::log::IbdbWriter writer(dest);
    LogT Logxx(filename);
    IbdbWriter* writer = Logxx.writer_;
    Slice data("MagnetoWang");
    LOG(INFO)<< data.data();
    writer->AddRecord(data);
    writer->AddRecord(data);
    writer->AddRecord(data);
    writer->AddRecord(data);
}

} // log
} // ibdb

int main(int argc, char** argv) {
    // ibdb::base::GlogInit();
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO,"/Users/magnetowang/Documents/GitHub/IcebergDB/ibdb/log/");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}