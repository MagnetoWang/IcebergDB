#ifndef IBDB_STORAGE_SEGMENT_H
#define IBDB_STORAGE_SEGMENT_H
#include <mutex>
#include <atomic>
#include "base/noncopyable.h"
#include "base/skiplist.h"
#include "base/arena.h"
#include "base/slice.h"

using ibdb::base::Noncopyable;
using ibdb::base::SkipList;
using ibdb::base::Arena;
using ibdb::base::Slice;
DECLARE_uint32(skiplist_height);

namespace ibdb {
namespace storage {

struct SegmentOffset {
    uint64_t offset;
};

struct SegmentTimeStamp {
    uint64_t ts;
    SegmentTimeStamp() {}
    // SegmentTimeStamp(uint64_t timestamp) : ts(timestamp) {}
};

class SliceComparator {
public:
    int operator()(const Slice& a, const Slice& b) const {
        return a.compare(b);
    }
};

class TimeStampComparator {
public:
    int operator()(const SegmentTimeStamp& a, const SegmentTimeStamp& b) const {
        if (a.ts == b.ts) {
            return 0;
        }
        return ((a.ts > b.ts) ? 1 : -1);
    }
};

typedef Slice Feature;
// typedef std::map<Slice, uint64_t> Index;
typedef SkipList<Slice, SegmentOffset*, SliceComparator> Index;
typedef SkipList<SegmentTimeStamp, Index*, TimeStampComparator>  ValueEntry;
typedef SkipList<Slice, ValueEntry*, SliceComparator> Entries;
// TODO 添加多线程互斥元
class Segment {
public:
    Segment();
    Segment(uint8_t skiplist_height);
    ~Segment();
    void Put(const Slice& key, uint64_t timestamp, const Slice& value);
    bool Put(const Slice& key, const uint64_t timestamp, const Slice& value, const uint64_t offset);
    void Get(const Slice& key, uint64_t timestamp, const Slice& value);
    bool Contains(const Slice& key);
    // TODO it is unreasonable for functions design under below
    bool Contains(const Slice& key, const uint64_t timestamp);
    bool Contains(const Slice& key, const uint64_t timestamp, const Slice& value);
    bool Remove(const Slice& key);
    bool Remove(const Slice& key, const uint64_t timestamp);
    bool Remove(const Slice& key, const uint64_t timestamp, const Slice& value);
    bool BuildIndex(const Slice& key);

private:
    Entries* segment_;
    Arena arena_;
    std::atomic<uint64_t> entries_count_;
    std::atomic<uint64_t> memory_size_;
    uint8_t skiplist_height_;
    std::mutex mu_;
};

const static SliceComparator feature_comp;
const static TimeStampComparator ts_comp;
const static SliceComparator slice_comp;

Segment::Segment()
    :   entries_count_(0),
        memory_size_(0) {
    skiplist_height_ = (uint8_t)FLAGS_skiplist_height;
    segment_ = new Entries(feature_comp, &arena_);
}

Segment::Segment(uint8_t skiplist_height)
    :   entries_count_(0),
        memory_size_(0),
        skiplist_height_(skiplist_height) {
    segment_ = new Entries(feature_comp, &arena_);
}

Segment::~Segment() {
    delete segment_;
}

// is Contains key
bool Segment::Contains(const Slice& key) {
    return segment_->Contains(key);
}

// is Contains ts
bool Segment::Contains(const Slice& key, const uint64_t timestamp) {
    // std::lock_guard<std::mutex> lock(mu_);
    SegmentTimeStamp ts;
    ts.ts = timestamp;
    if (segment_->Contains(key)) {
        return ((segment_->GetValue(key)))->Contains(ts);
    } else {
        return false;
    }
}

// is Contains value
bool Segment::Contains(const Slice& key, const uint64_t timestamp, const Slice& value) {
    SegmentTimeStamp ts;
    ts.ts = timestamp;
    if (Contains(key, timestamp)) {
        return ((((segment_->GetValue(key)))->GetValue(ts)))->Contains(value);
    } else {
        return false;
    }
}

// remove key
bool Segment::Remove(const Slice& key) {
    if (Contains(key)) {
        return segment_->Remove(key);
    } else {
        LOG(WARNING) << "no key in segment";
        return false;
    }
}

// remove timestamp
bool Segment::Remove(const Slice& key, const uint64_t timestamp) {
    if (Contains(key, timestamp)) {
            SegmentTimeStamp ts;
            ts.ts = timestamp;
        return ((segment_->GetValue(key)))->Remove(ts);
    } else {
        LOG(WARNING) << "no timestamp in segment which key is " << key.data();
        return false;
    }
}

// remove value
bool Segment::Remove(const Slice& key, const uint64_t timestamp, const Slice& value) {
    if (Contains(key, timestamp, value)) {
        SegmentTimeStamp ts;
        ts.ts = timestamp;
        return ((((segment_->GetValue(key)))->GetValue(ts)))->Remove(value);
    } else {
        LOG(WARNING) << "no value in segment which key is " << key.data() << "and ts = " << timestamp;
        return false;
    }
}

// build key index 
// but it is not timestamp index
bool Segment::BuildIndex(const Slice& key) {
    if (!Contains(key)) {
        TimeStampComparator ts_cmp;
        Arena arena;
        ValueEntry* value_entry = new ValueEntry(ts_cmp, &arena);
        segment_->Insert(key, value_entry);
        return true;
    } else {
        return false;
    }
}

// find key = value in timestamp
// value is slice type which is index key
// bool Segment::Get(const Slice& key, const uint64_t timestamp, const Slice& key_value, Slice& list) {
//     if (Contains(key, timestamp, value)) {
//         // TODO get data from disk by offset
//         list = nullptr;
//         return true;
//     }
//     return false;
// }

bool Segment::Put(const Slice& key, const uint64_t timestamp, const Slice& value, const uint64_t offset) {
    if (Contains(key)) {
        
    } else {

    }
    return true;
}

} // ibdb
} // storage

#endif