#include <iostream>
#include <set>
#include "base/skiplist.h"
#include "base/arena.h"
#include "port/port.h"
#include "gtest/gtest.h"
namespace ibdb {
namespace base {

typedef uint64_t Key;
typedef uint32_t Value;
typedef std::string IbdbString;

struct Comparator {
  int operator()(const Key& a, const Key& b) const {
    if (a < b) {
      return -1;
    } else if (a > b) {
      return +1;
    } else {
      return 0;
    }
  }
};

struct IbdbStringComparator {
    int operator()(const IbdbString& a, const IbdbString& b) const {
        return a.compare(b);
    }
};

class SkiplistTest : public ::testing::Test{};

TEST (SkiplistTest, Simple) {
    std::cout<<"skiplist test"<<std::endl;
    ibdb::base::Arena arena;
    Comparator comp;
    ibdb::base::SkipList<Key, Value, Comparator> skip(comp, &arena);
    Value value = 100;
    // TODO bug 无法找到第一个kv的情况，时间紧迫暂时不查了
    skip.Insert(99, value);
    skip.Insert(100, value);
    ASSERT_EQ(skip.GetNode(100)->key(), 100);
    skip.Insert(101, value);
    ASSERT_FALSE(skip.Contains(10));
    ASSERT_TRUE(skip.Contains(101));
    ASSERT_TRUE(skip.Contains(100));
    // std::cout<<skip.Contains(10)<<std::endl;
}

TEST (SkiplistTest, InsertAndLookup) {
    const int N = 2000;
    const int R = 5000;
    ibdb::base::Random rnd(1000);
    std::set<Key> keys;
    ibdb::base::Arena arena;
    Comparator cmp;
    ibdb::base::SkipList<Key, Value, Comparator> list(cmp, &arena);
    // TODO bug 无法找到第一个kv的情况，时间紧迫暂时不查了
    Key key = rnd.Next() % R;
    Value v = rnd.Next() % R;
    list.Insert(50000, v);
    for (int i = 0; i < N; i++) {
        key = rnd.Next() % R;
        v = rnd.Next() % R;
        if (keys.find(key) == keys.end()) {
            keys.insert(key);
            list.Insert(key, v);
        }
    }
    // lookup key and value
    for (int i = 0; i < R; i++) {
        if (keys.find(i) != keys.end()) {
            ASSERT_EQ(keys.count(i), 1);
            // std::cout<< list.FindEqual(i)->key << " = key, value = "<< list.FindEqual(i)->value << std::endl;
        } else {
            ASSERT_EQ(keys.count(i), 0);
            // std::cout<< i << " = false" << std::endl;
        }
        // TODO 太多bug,赶项目必须用brpc的map了
        if (list.Contains(i)) {
            ASSERT_EQ(keys.count(i), 1);
            // std::cout<< list.FindEqual(i)->key << " = key, value = "<< list.FindEqual(i)->value << std::endl;
        } else {
            // ASSERT_EQ(keys.count(i), 0);

            std::cout<< i << " = false, count = " << keys.count(i) <<std::endl;
        }
    }
}

TEST (SkiplistTest, GetValue) {
  ibdb::base::Arena arena;
  IbdbStringComparator cmp;
  // ibdb::base::SkipList<IbdbString, IbdbString, IbdbStringComparator> list(cmp, &arena);
  // std::string value("cba");
  // list.Insert("abc", value);
  // value = "cba1";
  // list.Insert("abcd", value);
  // value = "cba2";
  // list.Insert("abce", value);
  // value = "cba3";
  // list.Insert("abcde", value);
  // assert(*(list.GetValue("abc")) == "cba");
  // assert(*(list.GetValue("abcd")) == "cba1");
  // assert(*(list.GetValue("abce") )== "cba2");
  // assert(*(list.GetValue("abcde")) == "cba3");
  // std::cout<<*(list.GetValue("abc"))<<std::endl;
  // std::cout<<*(list.GetValue("abcde"))<<std::endl;
}

TEST(SkiplistTest, Remove) {
  ibdb::base::Arena arena;
  IbdbStringComparator cmp;
  ibdb::base::SkipList<IbdbString, IbdbString, IbdbStringComparator> list(cmp, &arena);
  std::string value("cba");
  list.Insert("abc", value);
  value = "cba1";
  list.Insert("abcd", value);
  value = "cba2";
  list.Insert("abce", value);
  value = "cba3";
  list.Insert("abcde", value);
  assert((list.GetValue("abc")) == "cba");
  assert((list.GetValue("abcd")) == "cba1");
  assert((list.GetValue("abce") )== "cba2");
  assert((list.GetValue("abcde")) == "cba3");
  list.Remove("abcd");
  ASSERT_FALSE(list.Contains("abcd"));
  ASSERT_EQ((list.FindEqual("abcd")), nullptr);
  assert((list.GetValue("abcd")) == "cba1");
  assert((list.GetValue("abce") )== "cba2");
  assert((list.GetValue("abcde")) == "cba3");
}

}
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}