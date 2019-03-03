#ifndef IBDB_LOG_FORMAT_H
#define IBDB_LOG_FORMAT_H
// #include <unistd.h>
#include "base/noncopyable.h"
#include "base/status.h"
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
    virtual ~WritableFile();

    virtual ibdb::base::Status Append(const ibdb::base::Slice& data) = 0;
    virtual ibdb::base::Status Close() = 0;
    virtual ibdb::base::Status Flush() = 0;
    virtual ibdb::base::Status Sync() = 0;
    virtual uint64_t GetSize() = 0;
protected:
    uint64_t filesize_;
};

static WritableFile* NewWritableFile(const std::string& fname, FILE* f);

// A file abstraction for reading sequentially through a file
class SequentialFile : ibdb::base::Noncopyable {
public:
    SequentialFile() = default;
    SequentialFile(const std::string& fname, FILE* f);
    virtual ~SequentialFile();

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

static SequentialFile* NewSeqFile(const std::string& fname, FILE* f);

// A file abstraction for randomly reading the contents of a file.
class RandomAccessFile : ibdb::base::Noncopyable {
public:
    RandomAccessFile() = default;
    virtual ~RandomAccessFile();

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



} // log
} // ibdb

#endif // IBDB_LOG_FORMAT_H


