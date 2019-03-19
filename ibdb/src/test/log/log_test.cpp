#include "log/format.h"
#include "log/log_reader.h"
#include "log/log_writer.h"

#include "base/utils.h"
#include "base/random.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include <gflags/gflags.h>

using ibdb::log::WritableFile;
using ibdb::base::Random;

namespace ibdb {
namespace log {

// Construct a string of the specified length made out of the supplied
// partial string.
static std::string BigString(const std::string& partial_string, size_t n) {
  std::string result;
  while (result.size() < n) {
    result.append(partial_string);
  }
  result.resize(n);
  return result;
}

// Construct a string from a number
static std::string NumberString(int n) {
  char buf[50];
  snprintf(buf, sizeof(buf), "%d.", n);
  return std::string(buf);
}

// Return a skewed potentially long string
static std::string RandomSkewedString(int i, Random* rnd) {
  return BigString(NumberString(i), rnd->Skewed(17));
}

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
    int offset = 10;
    std::string create_string("create table_name ts_name,uint_64,isIndex col_name,type,isIndex  col_name,type,isIndex");
    std::string offset_string = std::to_string(offset) + " " + create_string;
    Slice create(offset_string);
    LOG(INFO)<< create.data();
    writer->AddRecord(create);
    writer->AddRecord(create);
    writer->AddRecord(create);
    writer->AddRecord(create);
}

TEST(LogTest, ReadRecord) {
    std::string filename("addrecordtest.txt");
    // FILE* writestream_ = fopen(filename.c_str(), "w");
    // WritableFile* dest = ibdb::log::NewWritableFile(filename, writestream_);
    // Writer* writer = new Writer(dest);
    // ibdb::log::IbdbWriter writer(dest);
    LogT Logxx(filename);
    IbdbWriter* writer = Logxx.writer_;
    int offset = 10;
    std::string create_string("create table_name ts_name,uint_64,isIndex col_name,type,isIndex  col_name,type,isIndex");
    std::string offset_string = std::to_string(offset) + " " + create_string;
    Slice create(offset_string);
    LOG(INFO)<< create.data();
    writer->AddRecord(create);
    writer->AddRecord(create);
    writer->AddRecord(create);
    writer->AddRecord(create);
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