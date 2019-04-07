#ifndef IBDB_STORAGE_DISK_H
#define IBDB_STORAGE_DISK_H
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <iostream>

#include "log/log_reader.h"
#include "log/log_writer.h"
#include "log/format.h"
#include "base/coding.h"

#include "gflags/gflags.h"
#include "glog/logging.h"

using ibdb::log::SequentialFile;
using ibdb::log::RandomAccessFile;
using ibdb::log::Reader;
using ibdb::log::WriteHandle;
using ibdb::log::WritableFile;
using ibdb::log::Limiter;
using ibdb::base::Status;

DECLARE_uint64(limiter_max_required);
namespace ibdb {
namespace storage {

class WritableFileHandle {
public:
    explicit WritableFileHandle(const std::string& filename);
    WritableFileHandle(const std::string& filename, uint32_t current_pos);
    ~WritableFileHandle() {
        if (filestream_ != nullptr) {
            fclose(filestream_);
        }
    }

    Status Append(const Slice& slice) {
        return file_handle_->Append(slice);
    }

    Status Append(const Slice* const slice) {
        return file_handle_->Append(*slice);
    }

    // append message with offset.
    // format is likely offset + message.size() + message.
    Status Append(uint64_t offset, const Slice& slice) {
        char* offset_byte = new char[8];
        ibdb::base::EncodeFixed64(offset_byte, offset);
        char* size_byte = new char[4];
        ibdb::base::EncodeFixed32(size_byte, slice.size());
        Slice* log_data = new Slice(offset_byte, 8);
        Status s = Append(*log_data);
        if (!s.ok()) {
            return s;
        }
        log_data = new Slice(size_byte, 4);
        s = Append(*log_data);
        if (!s.ok()) {
            return s;
        }
        log_data = new Slice(slice.data(), slice.size());
        s = Append(*log_data);
        if (!s.ok()) {
            return s;
        }
        current_pos_ = current_pos_ + 12 + slice.size();
        return Status::OK();
    }

    Status Sync() {
        return file_handle_->Sync();
    }

    uint64_t GetSize() const {
        return file_handle_->GetSize();
    }

    uint32_t GetCurrentPos() const {
        return  current_pos_;
    }

private:
    std::string filename_;
    FILE* filestream_;
    uint32_t current_pos_;
    WritableFile* file_handle_;
};

// append log
WritableFileHandle::WritableFileHandle(const std::string& filename) 
    :   filename_(filename),
        filestream_(nullptr),
        current_pos_(0),
        file_handle_(nullptr) {
    filestream_ = fopen (filename_.c_str() , "a+");
    file_handle_ = ibdb::log::NewWritableFile(filename_, filestream_);
}

WritableFileHandle::WritableFileHandle(const std::string& filename, uint32_t current_pos)
    :   filename_(filename),
        filestream_(nullptr),
        current_pos_(current_pos),
        file_handle_(nullptr) {
        filestream_ = fopen (filename_.c_str() , "a+");
        file_handle_ = ibdb::log::NewWritableFile(filename_, filestream_);
}

// class ReaderHandle {
// private:
//     enum FileType {
//         kSequentialFile = 0,
//         kRandomAccessFile = 1
//     };
// public:
//     explicit ReaderHandle(std::string& filename, int file_type);
//     ReaderHandle(std::string& filename, int file_type, uint64_t offset);
//     ~ReaderHandle() {
//         delete file_handle_;
//         if (filestream_ != nullptr) {
//             filestream_ = nullptr;
//         }
//     }

//     Status ReadRecord(Slice* record) {
//         std::string scratch = nullptr;
//         if (file_handle_->ReadRecord(record, &scratch)) {
//             return Status::OK();
//         }
//         return  Status::IOError("Can't Read Record", strerror(errno));
//     }

//     uint64_t getLastOffset() {
//         return file_handle_->LastRecordOffset();
//     }

// private:
//     std::string filename_;
//     FILE* filestream_;
//     uint64_t offset_;
//     uint64_t record_size_;
//     uint64_t fd_;
//     int file_type_;
//     Limiter* limiter_;
//     RandomAccessFile* random_access_file_;
//     SequentialFile* sequential_file_;
//     Reader* file_handle_;
// };

// ReaderHandle::ReaderHandle(std::string& filename, int file_type)
//     :   filename_(filename),
//         offset_(0),
//         record_size_(0),
//         file_type_(file_type),
//         fd_(0),
//         limiter_(nullptr),
//         file_handle_(nullptr),
//         filestream_(nullptr) {
//             // filestream_ = fopen(filename.c_str(), "r+");
//             // fd_ = fileno(filestream_);
//             // limiter_ = new Limiter(FLAGS_limiter_max_required);
//             // file_ = ibdb::log::NewRandomAccessFile(filename_, fd_, limiter_);
//             // file_handle_ = new Reader(file_, nullptr, true, 0);
// }

// ReaderHandle::ReaderHandle(std::string& filename, int file_type, uint64_t offset)
//     :   filename_(filename),
//         offset_(0),
//         record_size_(0),
//         file_type_(file_type),
//         fd_(0),
//         limiter_(nullptr),
//         file_handle_(nullptr),
//         filestream_(nullptr) {
//             filestream_ = fopen(filename.c_str(), "r+");
//             fd_ = fileno(filestream_);
//             if (file_type_ == kSequentialFile) {

//             }
//             if (file_type_ == kRandomAccessFile) {
//                 // limiter_ = new Limiter(FLAGS_limiter_max_required);
//                 // file_ = ibdb::log::NewRandomAccessFile(filename_, fd_);
//                 // file_handle_ = new Reader(sequential_file_, nullptr, true, offset);
//             }
// }

class RandomAccessFileHandle {
public:
    explicit RandomAccessFileHandle(std::string& filename);
    RandomAccessFileHandle(std::string& filename, uint64_t offset);
    ~RandomAccessFileHandle() {
        delete random_access_file_;
        if (filestream_ != nullptr) {
            filestream_ = nullptr;
        }
    }

    Status Read(uint64_t offset, size_t n, Slice* const result) {
        char* scratch = nullptr;
        current_offset_ += n;
        return random_access_file_->Read(offset, n, result, scratch);
    }

    Status GetMessage(uint64_t offset, Slice* const result) {
        int offset_length = 8;
        int message_length = 4;
        char* scratch = new char[4];
        offset += offset_length;
        ssize_t read_size = ::pread(fd_, scratch, message_length, static_cast<off_t>(offset));
        if (read_size != 4) {
            Slice io_error("read_size is not equal message_length");
            LOG(ERROR) << io_error.data();
            return Status::IOError(io_error);
        }
        int message_size = ibdb::base::DecodeFixed32(scratch);
        scratch = new char[message_size];
        offset += message_length;
        read_size = ::pread(fd_, scratch, message_size, static_cast<off_t>(offset));
        if (read_size != message_size) {
            Slice io_error("read_size is not equal message_size");
            LOG(ERROR) << io_error.data();
            return Status::IOError(io_error);
        }
        
        *result = Slice(scratch, message_size);
        return Status::OK();
    }

    Status GetMessageSize(uint64_t offset, int result) {
        int offset_length = 8;
        int message_length = 4;
        char* scratch = new char[4];
        offset += offset_length;
        ssize_t read_size = ::pread(fd_, scratch, message_length, static_cast<off_t>(offset));
        if (read_size != 4) {
            Slice io_error("read_size is not equal message_length");
            LOG(ERROR) << io_error.data();
            return Status::IOError(io_error);
        }
        result = ibdb::base::DecodeFixed32(scratch);
        return Status::OK();
    }

    uint64_t GetCurrentOffset() const {
        return current_offset_;
    }

private:
    std::string filename_;
    FILE* filestream_;
    // uint64_t offset_;
    uint64_t current_offset_;
    int fd_;
    Limiter* limiter_;
    RandomAccessFile* random_access_file_;
};

RandomAccessFileHandle::RandomAccessFileHandle(std::string& filename)
    :   filename_(filename),
        current_offset_(0) {
            filestream_ = fopen(filename.c_str(), "r+");
            assert(filestream_ != nullptr);
            fd_ = fileno(filestream_);
            assert(fd_ != -1);
            Limiter limiter(FLAGS_limiter_max_required);
            limiter_ = &limiter;
            assert(limiter_ != nullptr);
            random_access_file_ = ibdb::log::NewRandomAccessFile(filename_, fd_, limiter_);
            assert(random_access_file_ != nullptr);
        }

RandomAccessFileHandle::RandomAccessFileHandle(std::string& filename, uint64_t offset)
    :   filename_(filename),
        current_offset_(offset) {
            filestream_ = fopen(filename.c_str(), "r+");
            assert(filestream_ != nullptr);
            fd_ = fileno(filestream_);
            limiter_ = new Limiter(FLAGS_limiter_max_required);
            assert(limiter_ != nullptr);
            random_access_file_ = ibdb::log::NewRandomAccessFile(filename_, fd_, limiter_);
            assert(random_access_file_ != nullptr);
        }

} // ibdb
} // storage

#endif // IBDB_STORAGE_DISK_H