#ifndef IBDB_LOG_FORMAT_H
#define IBDB_LOG_FORMAT_H
#include "base/noncopyable.h"
#include "base/status.h"
#include "base/slice.h"

#include <stdio.h>
#include <unistd.h>
// #include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <atomic>

#include "glog/logging.h"

using ibdb::base::Status;
using ibdb::base::Slice;

namespace ibdb {
namespace log {

enum RecordType {
    // Zero is reserved for preallocated files
    kZeroType = 0,
    kFullType = 1,
    // For fragments
    kFirstType = 2,
    kMiddleType = 3,
    kLastType = 4,
    // The end of log file
    kEofType = 5
};

static const int kMaxRecordType = kEofType;

static const int kBlockSize = 4 * 1024;

// Header is checksum (4 bytes), length (2 bytes), type (1 byte).
static const int kHeaderSize = 4 + 2 + 1;

// A file abstraction for sequential writing.  The implementation
// must provide buffering since callers may append small fragments
// at a time to the file.
class WritableFile : ibdb::base::Noncopyable {
public:
    WritableFile() = default;
    virtual ~WritableFile() = default;

    virtual ibdb::base::Status Append(const ibdb::base::Slice& data) = 0;
    virtual ibdb::base::Status Close() = 0;
    virtual ibdb::base::Status Flush() = 0;
    virtual ibdb::base::Status Sync() = 0;
    virtual uint64_t GetSize() = 0;
protected:
    uint64_t filesize_;
};

// A file abstraction for reading sequentially through a file
class SequentialFile : ibdb::base::Noncopyable {
public:
    SequentialFile() = default;
    SequentialFile(const std::string& fname, FILE* f);
    virtual ~SequentialFile() {};

    // Read up to "n" bytes from the file.  "scratch[0..n-1]" may be
    // written by this routine.  Sets "*result" to the data that was
    // read (including if fewer than "n" bytes were successfully read).
    // May set "*result" to point at data in "scratch[0..n-1]", so
    // "scratch[0..n-1]" must be live when "*result" is used.
    // If an error was encountered, returns a non-OK status.
    //
    // REQUIRES: External synchronization
    virtual Status Read(size_t n, Slice* result, char* scratch) = 0;

    // Skip "n" bytes from the file. This is guaranteed to be no
    // slower that reading the same data, but may be faster.
    //
    // If end of file is reached, skipping will stop at the end of the
    // file, and Skip will return OK.
    //
    // REQUIRES: External synchronization
    virtual Status Skip(uint64_t n) = 0;
};

// A file abstraction for randomly reading the contents of a file.
class RandomAccessFile : ibdb::base::Noncopyable {
public:
    RandomAccessFile() = default;
    virtual ~RandomAccessFile() {};

    // Read up to "n" bytes from the file starting at "offset".
    // "scratch[0..n-1]" may be written by this routine.  Sets "*result"
    // to the data that was read (including if fewer than "n" bytes were
    // successfully read).  May set "*result" to point at data in
    // "scratch[0..n-1]", so "scratch[0..n-1]" must be live when
    // "*result" is used.  If an error was encountered, returns a non-OK
    // status.
    //
    // Safe for concurrent use by multiple threads.
    virtual Status Read(uint64_t offset, size_t n, Slice* result,
                        char* scratch) const = 0;
    virtual const int GetFd() const = 0;
};

class Logger {
public:
    Logger() = default;

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    virtual ~Logger();

    // Write an entry to the log file with the specified format.
    virtual void Logv(const char* format, va_list ap) = 0;
};

class  FileLock {
public:
    FileLock() = default;

    FileLock(const FileLock&) = delete;
    FileLock& operator=(const FileLock&) = delete;

    virtual ~FileLock();
};

// implementation
static Status IOError(const std::string& context, int err_number) {
    return Status::IOError(context, strerror(err_number));
}

static Status IOOk() {
    return Status::OK();
}

class UnixWritableFile final : public WritableFile {
public:
    UnixWritableFile(const std::string& filename, FILE* filestream) : filename_(filename), filestream_(filestream) {}
    ~UnixWritableFile() {
        if (filestream_ != NULL) {
            fclose(filestream_);
        }
    }

    Status Append(const Slice& data) {
        size_t r = fwrite(data.data(), 1, data.size(), filestream_);
        // 验证正确性
        if (r < 0 && r != data.size()) {
            return IOError(filename_, errno);
        }
        filesize_ += r;
        return IOOk();
    }

    Status Close() {
        if (fclose(filestream_) != 0) {
            filestream_ = nullptr;
            return IOError(filename_, errno);
        }
        return IOOk();
    }

    Status Flush() {
        if (fflush(filestream_) != 0) {
            filestream_ = nullptr;
            return IOError(filename_, errno);
        }
        return IOOk();
    }

    Status Sync() {
        if (fflush(filestream_) != 0 || fsync(fileno(filestream_)) != 0) {
            filestream_ = nullptr;
            return IOError(filename_, errno);
        }
        return IOOk();
    }

    uint64_t GetSize() {
        return filesize_;
    }

private:
    std::string filename_;
    FILE* filestream_;
};

WritableFile* NewWritableFile(const std::string& fname, FILE* f) {
    return new UnixWritableFile(fname, f);
}

Status UnixError(const std::string& context, int error_number) {
    if (error_number == ENOENT) {
        return Status::NotFound(context, std::strerror(error_number));
    } else {
        return Status::IOError(context, std::strerror(error_number));
    }
}

// Helper class to limit resource usage to avoid exhaustion.
// Currently used to limit read-only file descriptors and mmap file usage
// so that we do not run out of file descriptors or virtual memory, or run into
// kernel performance problems for very large databases.
class Limiter {
public:
    // Limit maximum number of resources to |max_acquires|.
    Limiter(int max_acquires) : acquires_allowed_(max_acquires) {}

    Limiter(const Limiter&) = delete;
    Limiter operator=(const Limiter&) = delete;

    // If another resource is available, acquire it and return true.
    // Else return false.
    bool Acquire() {
        int old_acquires_allowed =
            acquires_allowed_.fetch_sub(1, std::memory_order_relaxed);

        if (old_acquires_allowed > 0)
        return true;

        acquires_allowed_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    // Release a resource acquired by a previous call to Acquire() that returned
    // true.
    void Release() {
        acquires_allowed_.fetch_add(1, std::memory_order_relaxed);
    }

private:
    // The number of available resources.
    //
    // This is a counter and is not tied to the invariants of any other class, so
    // it can be operated on safely using std::memory_order_relaxed.
    std::atomic<int> acquires_allowed_;
};


// Implements sequential read access in a file using read().
//
// Instances of this class are thread-friendly but not thread-safe, as required
// by the SequentialFile API.
class UnixSequentialFile final : public SequentialFile {
public:
    UnixSequentialFile(std::string filename, int fd)
        : fd_(fd), filename_(filename) {}
    ~UnixSequentialFile() override { close(fd_); }

    Status Read(size_t n, Slice* result, char* scratch) override {
        Status status;
        while (true) {
        ssize_t read_size = read(fd_, scratch, n);
        if (read_size < 0) {  // Read error.
            if (errno == EINTR) {
            continue;  // Retry
            }
            status = UnixError(filename_, errno);
            break;
        }
        *result = Slice(scratch, read_size);
        break;
        }
        return status;
    }

    Status Skip(uint64_t n) override {
        if (::lseek(fd_, n, SEEK_CUR) == static_cast<off_t>(-1)) {
        return UnixError(filename_, errno);
        }
        return Status::OK();
    }

private:
    const int fd_;
    const std::string filename_;
};

SequentialFile* NewSequentialFile(const std::string& fname, int fd) {
    return new UnixSequentialFile(fname, fd);
}

// Implements random read access in a file using pread().
//
// Instances of this class are thread-safe, as required by the RandomAccessFile
// API. Instances are immutable and Read() only calls thread-safe library
// functions.
class PosixRandomAccessFile final : public RandomAccessFile {
public:
    // The new instance takes ownership of |fd|. |fd_limiter| must outlive this
    // instance, and will be used to determine if .
    PosixRandomAccessFile(std::string filename, int fd, Limiter* fd_limiter)
        : has_permanent_fd_(fd_limiter->Acquire()),
        fd_(has_permanent_fd_ ? fd : -1),
        fd_limiter_(fd_limiter),
        filename_(std::move(filename)) {
        if (!has_permanent_fd_) {
            assert(fd_ == -1);
            ::close(fd);  // The file will be opened on every read.
        }
    }

    ~PosixRandomAccessFile() override {
        if (has_permanent_fd_) {
            assert(fd_ != -1);
            ::close(fd_);
            fd_limiter_->Release();
        }
    }

    Status Read(uint64_t offset, size_t n, Slice* result,
                char* scratch) const override {
        int fd = fd_;
        if (!has_permanent_fd_) {
            fd = ::open(filename_.c_str(), O_RDONLY);
            if (fd < 0) {
                return UnixError(filename_, errno);
            }
        }

        assert(fd != -1);

        Status status;
        ssize_t read_size = ::pread(fd, scratch, n, static_cast<off_t>(offset));
        assert(read_size == n);
        *result = Slice(scratch, (read_size < 0) ? 0 : n);
        if (read_size < 0) {
            // An error: return a non-ok status.
            status = UnixError(filename_, errno);
        }
        if (!has_permanent_fd_) {
            // Close the temporary file descriptor opened earlier.
            assert(fd != fd_);
            ::close(fd);
        }
        return status;
    }
    const int GetFd() const override {
        return fd_;
    }
private:
    const bool has_permanent_fd_;  // If false, the file is opened on every read.
    const int fd_;  // -1 if has_permanent_fd_ is false.
    Limiter* const fd_limiter_;
    const std::string filename_;
};

RandomAccessFile* NewRandomAccessFile(std::string filename, int fd, Limiter* fd_limiter) {
    return new PosixRandomAccessFile(filename, fd, fd_limiter);
}

} // log
} // ibdb

#endif // IBDB_LOG_FORMAT_H


