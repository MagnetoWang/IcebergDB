#ifndef IBDB_STORAGE_SEGMENT_H
#define IBDB_STORAGE_SEGMENT_H
#include <mutex>

#include "base/noncopyable.h"
#include "base/skiplist.h"
#include "base/arena.h"
#include "base/slice.h"

using ibdb::base::Noncopyable;
using ibdb::base::SkipList;
using ibdb::base::Arena;
using ibdb::base::Slice;

namespace ibdb {
namespace storage {

class SliceComparator {
    int operator()(const Slice& a, const Slice& b) const {
        return a.compare(b);
    }
};

class TimeStampComparator {
    int operator()(const uint64_t& a, const uint64_t& b) {
        if (a == b) {
            return 0;
        }
        return ((a > b) ? 1 : -1);
    }
};


typedef Slice Feature;
typedef SkipList<Slice, uint64_t, SliceComparator> Index;
typedef SkipList<uint64_t, Index, TimeStampComparator>  ValueEntry;
typedef SkipList<Slice, ValueEntry, SliceComparator> Entries;

class Segment {
public:
    Segment(uint8_t skiplist_height);
    ~Segment();
    void Put(const Slice& key, uint64_t timestamp, const Slice& value);
    void Get(const Slice& key, uint64_t timestamp, const Slice& value);
    bool Contain(const Slice& key);
    bool Contain(const Slice& key, const uint64_t timestamp);
    bool Contain(const Slice& key, const uint64_t timestamp, const Slice& value);
    bool Delete();
    bool BuildIndex(const Slice& key);

private:
    Entries* segment_;
    Arena arena_;
    uint64_t entries_count_;
    uint64_t memory_size_;
    uint8_t skiplist_height_
    std::mutex mu_;
};

DECLARE_uint32(skiplist_height);
const static SliceComparator feature_comp;
const static TimeStampComparator ts_comp;
const static SliceComparator slice_comp;

Segment::Segment()
    :   entries_count_(0),
        memory_size_(0),
        mu_() {
    skiplist_height_ = (uint8_t)FLAGS_skiplist_height;
    segment_ = new Entries(feature_comp, &arena);
}

Segment::~Segment() {
    delete segment_;
}

// is contain key
bool Segment::Contain(const Slice& key) {
    return segment_->Contain(key);
}

// is contain ts
bool Segment::Contain(const Slice& key, const uint64_t timestamp) {
    if (segment_->Contain(key)) {
        return segment_->GetValue(key).Contains(timestamp);
    } else {
        return false;
    }
}

// is contain value
bool Segment::Contain(const Slice& key, const uint64_t timestamp, const Slice& value) {
    if(Contain(key, timestamp)) {
        return segment_->GetValue(key).GetValue(timestamp).Contains(value);
    } else {
        return false;
    }
}

bool Segment::BuildIndex(const Slice& key) {
    if (!Contain(key)) {
        TimeStampComparator ts_cmp;
        Arena arena;
        ValueEntry value_entry = new ValueEntry(ts_cmp, &arena);
        segment_->Insert(key, value_entry);
    } else {
        return false;
    }
}

// find key = value in timestamp
// value is slice type which is index key
bool Segment::Get(const Slice& key, const uint64_t timestamp, const Slice& key_value, Slice& list) {
    if (Contain(key, timestamp, value)) {
        // TODO get data from disk by offset
        list = nullptr;
        return true;
    }
    return false;
}

void Segment::Put(const Slice& key, const uint64_t timestamp, const Slice& value, const uint64_t offset) {
    if (Contain(key)) {
        
    } else {

    }
}

} // ibdb
} // storage

#endif