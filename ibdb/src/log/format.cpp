
#include "format.h"
#include "base/status.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
namespace ibdb {
namespace log {

static ibdb::base::Status IOError(const std::string& context, int err_number) {
    return ibdb::base::Status::IOError(context, strerror(err_number));
}

static ibdb::base::Status IOOk() {
    return ibdb::base::Status::OK();
}

static WritableFile* NewWritableFile(const std::string& fname, FILE* f) {
    return new UnixWritableFile(fname, f);
}

class UnixWritableFile : public WritableFile {
public:
    UnixWritableFile(const std::string& filename, FILE* filestream) : filename_(filename), filestream_(filestream) {}
    ~UnixWritableFile() {
        if (filestream_ != NULL) {
            fclose(filestream_);
        }
    }

    virtual ibdb::base::Status Append(const ibdb::base::Slice& data) {
        size_t r = fwrite(data.data(), 1, data.size(), filestream_);
        // 验证正确性
        if (r < 0 && r != data.size()) {
            return IOError(filename_, errno);
        }
        filesize_ += r;
        return IOOk();
    }

    virtual ibdb::base::Status Close() {
        if (fclose(filestream_) != 0) {
            filestream_ = nullptr;
            return IOError(filename_, errno);
        }
        return IOOk();
    }

    virtual ibdb::base::Status Flush() {
        if (fflush(filestream_) != 0) {
            filestream_ = nullptr;
            return IOError(filename_, errno);
        }
        return IOOk();
    }

    virtual ibdb::base::Status Sync() {
        if (fflush(filestream_) != 0 || fdatasync(fileno(filestream_)) != 0) {
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

} // log
} // ibdb