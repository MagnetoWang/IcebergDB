#ifndef IBDB_LOG_LOG_WEITER_H
#define IBDB_LOG_LOG_WEITER_H
#include "format.h"
#include "base/noncopyable.h"
#include "base/slice.h"
#include "base/status.h"
#include "base/crc32c.h"
using ibdb::base::Status;
using ibdb::base::Slice;
namespace ibdb {
namespace log {

class IbdbWriter : ibdb::base::Noncopyable {

public:
    // Create a writer that will append data to "*dest".
    // "*dest" must be initially empty.
    // "*dest" must remain live while this Writer is in use.
    IbdbWriter(WritableFile* dest);

    // Create a writer that will append data to "*dest".
    // "*dest" must have initial length "dest_length".
    // "*dest" must remain live while this Writer is in use.
    IbdbWriter(WritableFile* dest, uint64_t dest_length);

    ~IbdbWriter();

    Status AddRecord(const Slice& slice);
    // what is endlog?
    // Status EndLog();
private:
    WritableFile* dest_;
    int block_offset_;       // Current offset in block

    // crc32c values for all supported record types.  These are
    // pre-computed to reduce the overhead of computing the crc of the
    // record type stored in the header.
    uint32_t type_crc_[kMaxRecordType + 1];

    Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);
};

class WriteHandle {

public:
    WriteHandle(const std::string& filename, FILE* filestream, uint64_t dest_length = 0)
    :   filename_(filename),
        filestream_(filestream),
        wf_(nullptr),
        writer_(nullptr) {
            wf_ = ibdb::log::NewWritableFile(filename_, filestream_);
            writer_ = new IbdbWriter(wf_, dest_length);
        }

    ~WriteHandle() {
        delete wf_;
        delete writer_;
    }

    Status Write(const Slice& slice) {
        return writer_->AddRecord(slice);
    }

    Status Sync() {
        return wf_->Sync();
    }

    // Status EndLog() {
    //     return writer_->EndLog();
    // }

    uint64_t GetSize() const {
        return wf_->GetSize();
    }

private:
    std::string filename_;
    FILE* filestream_;
    WritableFile* wf_;
    IbdbWriter* writer_;
};

//implementation
static void InitTypeCrc(uint32_t* type_crc) {
  for (int i = 0; i <= kMaxRecordType; i++) {
    char t = static_cast<char>(i);
    type_crc[i] = ibdb::base::crc32c::Value(&t, 1);
  }
}

IbdbWriter::IbdbWriter(WritableFile* dest)
    : dest_(dest), block_offset_(0) {
    InitTypeCrc(type_crc_);
}

IbdbWriter::IbdbWriter(WritableFile* dest, uint64_t dest_length)
    : dest_(dest), block_offset_(dest_length % kBlockSize) {
    InitTypeCrc(type_crc_);
}

IbdbWriter::~IbdbWriter() {
    delete dest_;
}

Status IbdbWriter::AddRecord(const Slice& slice) {
  const char* ptr = slice.data();
  size_t left = slice.size();

  // Fragment the record if necessary and emit it.  Note that if slice
  // is empty, we still want to iterate once to emit a single
  // zero-length record
  Status s;
  bool begin = true;
  do {
    const int leftover = kBlockSize - block_offset_;
    assert(leftover >= 0);
    if (leftover < kHeaderSize) {
      // Switch to a new block
      if (leftover > 0) {
        // Fill the trailer (literal below relies on kHeaderSize being 7)
        assert(kHeaderSize == 7);
        dest_->Append(Slice("\x00\x00\x00\x00\x00\x00", leftover));
      }
      block_offset_ = 0;
    }

    // Invariant: we never leave < kHeaderSize bytes in a block.
    assert(kBlockSize - block_offset_ - kHeaderSize >= 0);

    const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
    const size_t fragment_length = (left < avail) ? left : avail;

    RecordType type;
    const bool end = (left == fragment_length);
    if (begin && end) {
      type = kFullType;
    } else if (begin) {
      type = kFirstType;
    } else if (end) {
      type = kLastType;
    } else {
      type = kMiddleType;
    }

    s = EmitPhysicalRecord(type, ptr, fragment_length);
    ptr += fragment_length;
    left -= fragment_length;
    begin = false;
  } while (s.ok() && left > 0);
  return s;
}

Status IbdbWriter::EmitPhysicalRecord(RecordType t, const char* ptr, size_t n) {
  assert(n <= 0xffff);  // Must fit in two bytes
  assert(block_offset_ + kHeaderSize + n <= kBlockSize);

  // Format the header
  char buf[kHeaderSize];
  buf[4] = static_cast<char>(n & 0xff);
  buf[5] = static_cast<char>(n >> 8);
  buf[6] = static_cast<char>(t);

  // Compute the crc of the record type and the payload.
  uint32_t crc = ibdb::base::crc32c::Extend(type_crc_[t], ptr, n);
  crc = ibdb::base::crc32c::Mask(crc);                 // Adjust for storage
  ibdb::base::EncodeFixed32(buf, crc);

  // Write the header and the payload
  Status s = dest_->Append(Slice(buf, kHeaderSize));
  if (s.ok()) {
    s = dest_->Append(Slice(ptr, n));
    if (s.ok()) {
      s = dest_->Flush();
    }
  }
  block_offset_ += kHeaderSize + n;
  return s;
}

} // log
} // ibdb

#endif // IBDB_LOG_LOG_WEITER_H