#include "storage/segment.h"
#include "storage/table.h"
#include "base/utils.h"

#include <atomic>
#include <ctime>

#include "gtest/gtest.h"
#include "glog/logging.h"
#include "gflags/gflags.h"
DECLARE_string(ibdb_log_dir);
DECLARE_string(db_root);
namespace ibdb {
namespace storage {

class SegmentTest {};

TEST(SegmentTest, Simple) {
    Segment segment(4);
    // delete segment;
}

TEST(SegmentTest, BuildKeyIndex) {
    Segment segment(4);
    Slice key("name");
    segment.BuildKeyIndex(key);
    ASSERT_EQ(segment.Contains(key), true);
}

TEST(SegmentTest, SegmentStruct) {
    // Segment segment(4);
    // Slice key("name");
    // ASSERT_EQ(segment.BuildKeyIndex(key), true);
    // ValueEntry* value_entry =  segment_->FindEqual(key)->value();
    // uint64_t timestamp = ibdb::base::GetMillisecondTimestamp();
    // SegmentTimeStamp ts;
    // ts.ts = timestamp;
    // Arena arena;
    // Index* index = new Index(slice_comp, &arena);
    // value_entry->Insert(ts, index);

}

TEST(SegmentTest, PutAndGet) {
    Segment segment(4);
    Slice key("name");
    segment.BuildKeyIndex(key);
    ASSERT_EQ(segment.Contains(key), true);
    uint64_t now_time = ibdb::base::GetMillisecondTimestamp();
    LOG(INFO) << now_time;
    Slice value("magneto");
    // LOG(INFO) << strlen("magneto");
    uint64_t offset = 100;
    bool result = segment.Put(key, now_time, value, offset);
    ASSERT_EQ(result, true);
    uint64_t get_offset = 0;
    result = segment.Get(key, now_time, value, get_offset);
    ASSERT_EQ(result, true);
    ASSERT_EQ(get_offset, offset);
    LOG(INFO) << get_offset;
}

//check whether the data is in memory or not
TEST(SegmentTest, MemPutAndGet) {
    Segment segment(4);
    Slice key("name");
    segment.BuildKeyIndex(key);
    ASSERT_EQ(segment.Contains(key), true);
    uint64_t number = 5;
    std::vector<std::string> value_list;
    std::vector<uint64_t> ts_list;
    uint64_t now_time = 0;
    for (int i = 0; i < number; i++) {
        std::string value_string = "value" + std::to_string(i);
        Slice value(value_string);
        value_list.push_back(value_string);
        uint64_t offset = i;
        // LOG(INFO) << value.data() << " now_time = " << now_time;
        bool result = segment.Put(key, now_time, value, offset);
        ASSERT_EQ(result, true);
        offset = 10;
        result = segment.Get(key, now_time, "value0", offset);
        ASSERT_EQ(result, true);
        ASSERT_EQ(0, offset);
    }
    // uint64_t now_time = i;
    for (int i = 0; i < number; i++) {
        
        std::string value_string = "value" + std::to_string(now_time);
        Slice value(value_string);
        uint64_t get_offset = 0;
        bool result = segment.Get(key, now_time, value, get_offset);
        ASSERT_EQ(result, true);
    }
}

// Put big data in segment and get all of data
TEST(SegmentTest, BenPutAndGet) {
    Segment segment(4);
    Slice key("name");
    segment.BuildKeyIndex(key);
    ASSERT_EQ(segment.Contains(key), true);
    uint64_t number = 100000;
    for (int i = 0; i < number; i++) {
        uint64_t now_time = i;
        std::string value_string = "value" + std::to_string(i);
        Slice value(value_string);
        uint64_t offset = i;
        bool result = segment.Put(key, now_time, value, offset);
        ASSERT_EQ(result, true);
        offset = 0;
        result = segment.Get(key, now_time, value, offset);
        ASSERT_EQ(result, true);
        ASSERT_EQ(i, offset);
    }
    for (int i = 0; i < number; i++) {
        uint64_t now_time = i;
        std::string value_string = "value" + std::to_string(i);
        Slice value(value_string);
        uint64_t get_offset = 0;
        bool result = segment.Get(key, now_time, value, get_offset);
        ASSERT_EQ(result, true);
        ASSERT_EQ(i, get_offset);
    }
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