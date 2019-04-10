#ifndef IBDB_STORAGE_SEGMENT_H
#define IBDB_STORAGE_SEGMENT_H
#include <mutex>
#include <atomic>
#include <chrono> // for timestamp

#include "base/noncopyable.h"
#include "base/skiplist.h"
#include "base/arena.h"
#include "base/slice.h"

#include "glog/logging.h"
#include "gflags/gflags.h"

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
};

class SliceComparator {
public:
    int operator()(const Slice& a, const Slice& b) const {
        return a.compare(b);
    }
};

// class TimeStampComparator {
// public:
//     int operator()(const SegmentTimeStamp& a, const SegmentTimeStamp& b) const {
//         if (a.ts == b.ts) {
//             return 0;
//         }
//         return ((a.ts > b.ts) ? 1 : -1);
//     }
// };

class TimeStampComparator {
public:
    int operator()(const uint64_t& a, const uint64_t& b) const {
        if (a == b) {
            return 0;
        }
        return ((a > b) ? 1 : -1);
    }
};

typedef Slice Feature;
// typedef std::map<Slice, uint64_t> Index;
typedef SkipList<Slice, uint64_t, SliceComparator> Index;
typedef SkipList<uint64_t, Index*, TimeStampComparator>  ValueEntry;
typedef SkipList<Slice, ValueEntry*, SliceComparator> Entries;
// TODO 添加多线程互斥元
class Segment {
public:
    Segment();
    Segment(uint8_t skiplist_height);
    ~Segment();
    // void Put(const Slice& key, uint64_t timestamp, const Slice& value);
    bool Put(const Slice& key, const uint64_t timestamp, const Slice& value, const uint64_t offset);
    bool Get(const Slice& key, const uint64_t timestamp, const Slice& value, uint64_t& offset);
    bool Contains(const Slice& key);
    // TODO it is unreasonable for functions design under below
    // bool Contains(const Slice& key, const uint64_t timestamp);
    // bool Contains(const Slice& key, const uint64_t timestamp, const Slice& value);
    bool Remove(const Slice& key);
    // bool Remove(const Slice& key, const uint64_t timestamp);
    // bool Remove(const Slice& key, const uint64_t timestamp, const Slice& value);
    bool BuildKeyIndex(const Slice& key);
    ValueEntry* NewValueEntry();
    Index* NewIndex();

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
// bool Segment::Contains(const Slice& key, const uint64_t timestamp) {
//     // std::lock_guard<std::mutex> lock(mu_);
//     SegmentTimeStamp ts;
//     ts.ts = timestamp;
//     if (segment_->Contains(key)) {
//         return ((segment_->GetValue(key)))->Contains(ts);
//     } else {
//         return false;
//     }
// }

// is Contains value
// bool Segment::Contains(const Slice& key, const uint64_t timestamp, const Slice& value) {
//     SegmentTimeStamp ts;
//     ts.ts = timestamp;
//     if (Contains(key, timestamp)) {
//         return ((((segment_->GetValue(key)))->GetValue(ts)))->Contains(value);
//     } else {
//         return false;
//     }
// }

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
// bool Segment::Remove(const Slice& key, const uint64_t timestamp) {
//     if (Contains(key, timestamp)) {
//             SegmentTimeStamp ts;
//             ts.ts = timestamp;
//         return ((segment_->GetValue(key)))->Remove(ts);
//     } else {
//         LOG(WARNING) << "no timestamp in segment which key is " << key.data();
//         return false;
//     }
// }

// remove value
// bool Segment::Remove(const Slice& key, const uint64_t timestamp, const Slice& value) {
//     if (Contains(key, timestamp, value)) {
//         SegmentTimeStamp ts;
//         ts.ts = timestamp;
//         return ((((segment_->GetValue(key)))->GetValue(ts)))->Remove(value);
//     } else {
//         LOG(WARNING) << "no value in segment which key is " << key.data() << "and ts = " << timestamp;
//         return false;
//     }
// }

// build key index 
// but it is not timestamp index
// TODO 局部函数会销毁valueEntry对象，后面在思考下该如何解决
bool Segment::BuildKeyIndex(const Slice& key) {
    if (!Contains(key)) {
        TimeStampComparator ts_cmp;
        ValueEntry* value_entry = new ValueEntry(ts_cmp, &arena_);
        char* data = new char[key.size()];
        memcpy(data, key.data(), key.size());
        Slice mem_key(data, key.size());
        segment_->Insert(mem_key, value_entry);
        return true;
    } else {
        LOG(ERROR) << "key is exitsted key[" << key.data() << "]";
        return false;
    }
}

ValueEntry* Segment::NewValueEntry() {
    TimeStampComparator ts_cmp;
    ValueEntry* value_entry = new ValueEntry(ts_cmp, &arena_);
    return value_entry;
}

Index* Segment::NewIndex() {
    SliceComparator slice_cmp;
    Index* index = new Index(slice_cmp, &arena_);
    return index;
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
    //TODO 为什么不能通过build方法来insert value?并且出现了segmentation fault问题
    // 假设key应该存在，如果不存在应该报错误。不能把随意建立索引
    // 建立索引的权利交给使用者才行！
    if (!Contains(key)) {
        LOG(INFO) << "key[" << key.data() << "] is not existed";
        return false;
        // assert(BuildKeyIndex(key));
    }
    ValueEntry* value_entry =  segment_->FindEqual(key)->value();
    uint64_t ts = timestamp;
    if(!value_entry->Contains(ts)) {
        LOG(INFO) << "new Index timestamp[" << ts << "]";
        Index* index = NewIndex();
        value_entry->Insert(ts, index);
    }
    Index* index = value_entry->FindEqual(ts)->value();
    if(index->Contains(value)) {
        LOG(INFO) << "value[" << index->FindEqual(value)->value() <<"] is existed";
        return false;
    }
    uint64_t segment_offset = offset;
    char* data = new char[value.size()];
    memcpy(data, value.data(), value.size());
    Slice new_value(data, value.size());

    index->Insert(new_value, segment_offset);
    LOG(INFO) << "key[" << key.data() 
              <<"] timestamp[" << ts 
              << "] value[" << new_value.data() 
              << "] offset[" << segment_offset <<"] is put successfully";
    return true;
}

bool Segment::Get(const Slice& key, const uint64_t timestamp, const Slice& value, uint64_t& offset) {
    if(!Contains(key)) {
        LOG(INFO) << "key[" << key.data() <<"] is not existed";
        return false;
    }
    ValueEntry* value_entry = segment_->FindEqual(key)->value();
    uint64_t ts = timestamp;
    if(!value_entry->Contains(ts)) {
        LOG(INFO) << "timestamp[" << timestamp <<"] is not existed";
        return false;
    }
    Index* index = value_entry->FindEqual(ts)->value();
    if(!index->Contains(value)) {
        LOG(INFO) << "value[" << value.data() <<"] is not existed";
        return false;
    }
    offset = index->FindEqual(value)->value();
    LOG(INFO) << "Get key[" << key.data() 
              <<"] timestamp[" << ts 
              << "] value[" << value.data() 
              << "] offset[" << offset <<"] is successed";
    return true;
}

} // ibdb
} // storage

#endif