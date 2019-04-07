#include "storage/disk.h"

#include <ctime>
// #include <cstring>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <atomic>

#include "base/utils.h"
#include "base/random.h"
#include "base/crc32c.h"

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"

using ibdb::storage::WritableFileHandle;
using ibdb::storage::RandomAccessFileHandle;
using ibdb::base::Random;

DECLARE_string(ibdb_log_dir);

namespace ibdb {
namespace storage {

class RandomAccessFileTest {};

TEST(RandomAccessFileTest, CreateFileTest) {
    std::string filename("AAACreateFileTest.txt");
    FILE* filestream = fopen(filename.c_str(), "a+r+");
    assert(filestream != nullptr);
    int fd = fileno(filestream);
    ASSERT_NE(fd, -1);
    ibdb::log::Limiter limiter(100);
    ibdb::log::Limiter* limiter_ptr = &limiter;
    ASSERT_NE(limiter_ptr, nullptr);
    // LOG(INFO) << limiter_ptr->Acquire();
    ASSERT_EQ(limiter_ptr->Acquire(), 1);
    RandomAccessFile* random_access_file = ibdb::log::NewRandomAccessFile(filename, fd, limiter_ptr);
    ASSERT_NE(random_access_file, nullptr);
}

TEST(RandomAccessFileTest, ReadInternTest) {
    std::string filename("AAAReadInternTest.txt");
    FILE* filestream = fopen(filename.c_str(), "ab+r+");
    assert(filestream != nullptr);

    WritableFileHandle writer_handle(filename);
    std::atomic<int> offset{0};
    for (int i = 0; i < 100; i++) {
        std::string create_string("create");
        std::string offset_string = std::to_string(offset) + " " + std::to_string(create_string.size()) + " " + create_string + "\n";
        Slice create(offset_string);
        writer_handle.Append(create);
        offset.fetch_add(1, std::memory_order_relaxed);
    }
    writer_handle.Sync();

    int fd = fileno(filestream);
    ASSERT_NE(fd, -1);
    ibdb::log::Limiter limiter(100);
    ibdb::log::Limiter* limiter_ptr = &limiter;
    ASSERT_NE(limiter_ptr, nullptr);
    // LOG(INFO) << limiter_ptr->Acquire();
    // RandomAccessFile* random_access_file = ibdb::log::NewRandomAccessFile(filename, fd, limiter_ptr);
    // log::PosixRandomAccessFile random_access_file(filename, fd, limiter_ptr);
    // LOG(INFO) << random_access_file->GetFd();
    // ASSERT_NE(random_access_file, nullptr);
    Slice* result = nullptr;
    int n = 100;
    char* scratch = (char*)malloc(sizeof(char) * n);
    uint64_t file_offset = 0;
    Status status;
    ssize_t read_size = ::pread(fd, scratch, n, static_cast<off_t>(file_offset));
    ASSERT_NE(read_size, -1);
    result = new Slice(scratch, (read_size < 0) ? 0 : read_size);
    // ASSERT_LT(read_size, 0);
    // LOG(INFO) << "read_size = " << read_size;
    // LOG(INFO) << result->data();
}

TEST(RandomAccessFileTest, ReadLogFormatTest) {
    std::string file("AAAReadLogFormatTest.txt");
    FILE* filestream = fopen(file.c_str(), "a+");
    // filestream->close();
    filestream = fopen(file.c_str(), "wrb+");

    ASSERT_NE(filestream, nullptr);
    // uint64_t number = 9000091212121;
    // LOG(INFO) << number;

    WritableFileHandle writer_handle(file);
    std::atomic<int> offset{0};
    for (uint64_t i = 0; i < 10; i++) {
        std::string message("create" + std::to_string(i));
        Slice* log_data;
        char* offset_byte = new char[8];
        char* size_byte = new char[4];
        // ibdb::base::EncodeFixed32(size_byte, message.size());
        // ibdb::base::EncodeFixed64(offset_byte, i);
        // ASSERT_EQ(ibdb::base::DecodeFixed32(size_byte), message.size());
        // ASSERT_EQ(ibdb::base::DecodeFixed64(offset_byte), i);
        // LOG(INFO) << message.size();
        log_data = new Slice(message);
        writer_handle.Append(i, *log_data);
        offset.fetch_add(1, std::memory_order_relaxed);
    }
    writer_handle.Sync();
    fclose(filestream);
    filestream = fopen(file.c_str(), "rb+");

    int fd = fileno(filestream);
    // Slice* result = nullptr;
    int offset_size = 8;
    int message_length = 4;
    char* scratch = nullptr;
    uint64_t file_offset = 0;
    Status status;
    for (int i = 0; i < 10; i++) {
        scratch = new char[offset_size];
        ssize_t read_size = ::pread(fd, scratch, offset_size, static_cast<off_t>(file_offset));
        // LOG(INFO) << ibdb::base::DecodeFixed64(scratch);
        ASSERT_EQ(ibdb::base::DecodeFixed64(scratch), i);
        scratch = new char[message_length];
        file_offset += offset_size;
        read_size = ::pread(fd, scratch, message_length, static_cast<off_t>(file_offset));
        // LOG(INFO) << ibdb::base::DecodeFixed32(scratch);
        ASSERT_EQ(ibdb::base::DecodeFixed32(scratch), 7);
        int message_size = ibdb::base::DecodeFixed32(scratch);
        file_offset += message_length;
        scratch = new char[message_size];
        // LOG(INFO) << message_size;
        read_size = ::pread(fd, scratch, message_size, static_cast<off_t>(file_offset));
        // LOG(INFO) << scratch;
        file_offset += message_size;
    }
}

class WritableFileHandleTes {};

TEST(WritableFileHandleTest, AppendTest) {
    std::string file("AAAWriterHandleTest.txt");
    WritableFileHandle writer_handle(file);
    std::atomic<int> offset{0};
    for (int i = 0; i < 10; i++) {
        std::string create_string("create\n");
        std::string offset_string = std::to_string(offset) + " " + std::to_string(create_string.size()) + " " + create_string;
        Slice create(offset_string);
        writer_handle.Append(create);
        offset.fetch_add(1, std::memory_order_relaxed);
    }
}

inline Slice LogFormat(uint64_t offset, std::string& message, FILE* filestream) {
    // uint64_t zero = 0;
    // char* zeros = new char[8];
    char* offset_byte = new char[8];
    char* size_byte = new char[4];
    ibdb::base::EncodeFixed32(size_byte, message.size());
    ibdb::base::EncodeFixed64(offset_byte, offset);
    size_t r = fwrite(offset_byte, 1, 8, filestream);
    assert(r == 8);
    r = fwrite(size_byte, 1, 4, filestream);
    assert(r == 4);
    r = fwrite(message.c_str(), 1, message.size(), filestream);
    fflush(filestream);

    int fd = fileno(filestream);
    Slice* result = nullptr;
    int offset_size = 8;
    int message_length = 4;
    char* scratch = (char*)malloc(sizeof(char) * offset_size);
    uint64_t file_offset = 0;
    Status status;
    for (int i = 0; i < 10; i++) {
        ssize_t read_size = ::pread(fd, scratch, offset_size, static_cast<off_t>(file_offset));
        file_offset += offset_size;
        // ssize_t flag = 8;
        // LOG(INFO) <<  read_size;
        // assert(read_size == flag);
        // LOG(INFO) << ibdb::base::DecodeFixed64(scratch);
        scratch = new char[4];
        read_size = ::pread(fd, scratch, message_length, static_cast<off_t>(file_offset));
        // assert(read_size == 4);
        LOG(INFO) << ibdb::base::DecodeFixed32(scratch);
        int message_size = ibdb::base::DecodeFixed32(scratch);
        file_offset += message_length;
        scratch = new char[message_size];
        read_size = ::pread(fd, scratch, message_size, static_cast<off_t>(file_offset));
        // assert(read_size == message_size);
        // LOG(INFO) << scratch;
        file_offset += message_size;
    }
    return *result;
}

TEST(WritableFileHandleTest, AppendLogFormatTest) {
    std::string file("AAAAppendLogFormatTest.txt");
    FILE* filestream = fopen(file.c_str(), "aw+");
    ASSERT_NE(filestream, nullptr);
    // uint64_t number = 9000091212121;
    // LOG(INFO) << number;

    WritableFileHandle writer_handle(file);
    std::atomic<int> offset{0};
    for (uint64_t i = 0; i < 10; i++) {
        std::string message("create" + std::to_string(i));
        Slice* log_data;
        char* offset_byte = new char[8];
        char* size_byte = new char[4];
        ibdb::base::EncodeFixed32(size_byte, message.size());
        ibdb::base::EncodeFixed64(offset_byte, offset);
        log_data = new Slice(offset_byte, 8);
        writer_handle.Append(log_data);
        log_data = new Slice(size_byte, 4);
        writer_handle.Append(log_data);
        log_data = new Slice(message);
        writer_handle.Append(log_data);
        offset.fetch_add(1, std::memory_order_relaxed);
    }
    writer_handle.Sync();
    fclose(filestream);
    filestream = fopen(file.c_str(), "r+");

    int fd = fileno(filestream);
    // Slice* result = nullptr;
    int offset_size = 8;
    int message_length = 4;
    char* scratch = (char*)malloc(sizeof(char) * offset_size);
    uint64_t file_offset = 0;
    Status status;
    for (int i = 0; i < 10; i++) {
        ssize_t read_size = ::pread(fd, scratch, offset_size, static_cast<off_t>(file_offset));
        file_offset += offset_size;
        // ssize_t flag = 8;
        // LOG(INFO) <<  read_size;
        // LOG(INFO) << ibdb::base::DecodeFixed64(scratch);
        ASSERT_EQ(ibdb::base::DecodeFixed64(scratch), i);
        scratch = new char[4];
        read_size = ::pread(fd, scratch, message_length, static_cast<off_t>(file_offset));
        // LOG(INFO) << ibdb::base::DecodeFixed32(scratch);
        ASSERT_EQ(ibdb::base::DecodeFixed32(scratch), 7);
        int message_size = ibdb::base::DecodeFixed32(scratch);
        file_offset += message_length;
        scratch = new char[message_size];
        read_size = ::pread(fd, scratch, message_size, static_cast<off_t>(file_offset));
        // LOG(INFO) << scratch;
        file_offset += message_size;
    }
}

TEST(WritableFileHandleTest, AppendOffset) {
    std::string file("AAAAppendOffsetTest.txt");
    FILE* filestream = fopen(file.c_str(), "arb+");
    assert(filestream != nullptr);

    // char* offset_byte = new char[8];
    // char* size_byte = new char[4];
    uint64_t offset = 9000091212121;
    Slice message("create");


    WritableFileHandle writer_handle(file);
    // ASSERT_NE(writer_handle, nullptr);
    writer_handle.Append(offset, message);
    writer_handle.Sync();

    RandomAccessFileHandle reader_handle(file);
    Slice* result = new Slice();
    reader_handle.GetMessage(0, result);
    // ASSEERT_EQ(result->data(), "create");
    ASSERT_NE(result, nullptr);
    // LOG(INFO) << result->data();
}

class RandomAccessFileHandleTest {};

TEST(RandomAccessFileHandleTest, GetMessage) {

}

TEST(RandomAccessFileHandleTest, GetMessageSize) {
    
}

class DiskTest {};

TEST(DiskTest, RandomAccessFileHandleTest) {
    std::string file("AAAWriterHandleTest.txt");
    // LOG(INFO) << "RandomAccessFileHandle is ok";
    // LOG(INFO) << "RandomAccessFileHandle is ok";
    ibdb::log::Limiter limiter(100);
    // ibdb::log::Limiter* limiter_ = &limiter;
    // LOG(INFO) << "new Limiter";
    // LOG(INFO) << limiter_->Acquire();

    // // RandomAccessFile* random_access_file
    // RandomAccessFileHandle reader_handle(file);
    // // // LOG(INFO) << "RandomAccessFileHandle is ok";
    // Slice* result = nullptr;
    // reader_handle.Read(0, 10, result);
    // LOG(INFO) << reader_handle.GetCurrentOffset();
    // LOG(INFO) << result->data();
}



} // storage
} // ibdb

int main(int argc, char** argv) {
    // ibdb::base::GlogInit();
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, FLAGS_ibdb_log_dir.c_str());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}