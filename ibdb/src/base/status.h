#ifndef IBDB_BASE_STATUS_H
#define IBDB_BASE_STATUS_H
#include "slice.h"
#include "port/port.h"
namespace ibdb {
namespace base {

class Status
{
public:
    // Create a success status.
    Status() noexcept : state_(nullptr) { }
    ~Status() { delete[] state_; }

    Status(const Status& rhs);
    Status& operator=(const Status& rhs);

  // Return a success status.
  static Status OK() { return Status(); }

  // Return error status of an appropriate type.
  static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kNotFound, msg, msg2);
  }
  static Status Corruption(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kCorruption, msg, msg2);
  }
  static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kNotSupported, msg, msg2);
  }
  static Status InvalidArgument(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kInvalidArgument, msg, msg2);
  }
  static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kIOError, msg, msg2);
  }

  // Returns true iff the status indicates success.
  bool ok() const { return (state_ == nullptr); }

  // Returns true iff the status indicates a NotFound error.
  bool IsNotFound() const { return code() == kNotFound; }

  // Returns true iff the status indicates a Corruption error.
  bool IsCorruption() const { return code() == kCorruption; }

  // Returns true iff the status indicates an IOError.
  bool IsIOError() const { return code() == kIOError; }

  // Returns true iff the status indicates a NotSupportedError.
  bool IsNotSupportedError() const { return code() == kNotSupported; }

  // Returns true iff the status indicates an InvalidArgument.
  bool IsInvalidArgument() const { return code() == kInvalidArgument; }

  // Return a string representation of this status suitable for printing.
  // Returns the string "OK" for success.
  std::string ToString() const;

private:
    // OK status has a null state_.  Otherwise, state_ is a new[] array
    // of the following form:
    //    state_[0..3] == length of message
    //    state_[4]    == code
    //    state_[5..]  == message
    const char* state_;

    enum Code {
        kOk = 0,
        kNotFound = 1,
        kCorruption = 2,
        kNotSupported = 3,
        kInvalidArgument = 4,
        kIOError = 5,
        kWaitRecord = 6,
        kEof = 7
    };

    Code code() const {
        return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
    }

    Status(Code code, const Slice& msg, const Slice& msg2);
    const char* CopyState(const char* s);
};

inline Status::Status(const Status& rhs) {
    state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
}

const char* Status::CopyState(const char* state) {
    uint32_t size;
    memcpy(&size, state, sizeof(size));
    char* result = new char[size + 5];
    memcpy(result, state, size + 5);
    return result;
}

Status::Status(Code code, const Slice& msg, const Slice& msg2) {
    assert(code != kOk);
    const uint32_t len1 = msg.size();
    const uint32_t len2 = msg2.size();
    const uint32_t size = len1 + (len2 ? (2 + len2) : 0);
    char* result = new char[size + 5];
    memcpy(result, &size, sizeof(size));
    result[4] = static_cast<char>(code);
    memcpy(result + 5, msg.data(), len1);
    if (len2) {
      result[5 + len1] = ':';
      result[6 + len1] = ' ';
      memcpy(result + 7 + len1, msg2.data(), len2);
    }
    state_ = result;
}

inline Status& Status::operator=(const Status& rhs) {
    // The following condition catches both aliasing (when this == &rhs),
    // and the common case where both rhs and *this are ok.
    if (state_ != rhs.state_) {
        delete[] state_;
        state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
    }
    return *this;
}

std::string Status::ToString() const {
    if (state_ == NULL) {
        return "OK";
    } else {
        char tmp[30];
        const char* type;
        switch (code()) {
            case kOk:
                type = "OK";
                break;
            case kNotFound:
                type = "NotFound: ";
                break;
            case kCorruption:
                type = "Corruption: ";
                break;
            case kNotSupported:
                type = "Not implemented: ";
                break;
            case kInvalidArgument:
                type = "Invalid argument: ";
                break;
            case kIOError:
                type = "IO error: ";
                break;
            case kWaitRecord:
                type = "Wait record";
                break;
            case kEof:
                type = "End of file";
                break;
            default:
                snprintf(tmp, sizeof(tmp), "Unknown code(%d): ",
                   static_cast<int>(code()));
                type = tmp;
                break;
      }
      std::string result(type);
      uint32_t length;
      memcpy(&length, state_, sizeof(length));
      result.append(state_ + 5, length);
      return result;
    }
}

} // base
} // ibdb

#endif // IBDB_BASE_STATUS_H