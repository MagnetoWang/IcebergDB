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

class SkiplistTest : public ::testing::Test{

};

TEST (SkiplistTest, Simple) {
    std::cout<<"skiplist test"<<std::endl;
    ibdb::base::Arena arena;
    Comparator comp;
    // ibdb::base::SkipList<int, Comparator> skip;
    ibdb::base::SkipList<Key, Value, Comparator> skip(comp, &arena);
    assert(!skip.Contains(10));
    std::cout<<skip.Contains(10)<<std::endl;
}

TEST (SkiplistTest, InsertAndLookup) {
    const int N = 2000;
    const int R = 5000;
    ibdb::base::Random rnd(1000);
    std::set<Key> keys;
    ibdb::base::Arena arena;
    Comparator cmp;
    ibdb::base::SkipList<Key, Value, Comparator> list(cmp, &arena);
    for (int i = 0; i < N; i++) {
        Key key = rnd.Next() % R;
        Value v = rnd.Next() % R;
        if (keys.insert(key).second) {
            list.Insert(key, v);
        }
    }
    // lookup key and value
    for (int i = 0; i < R; i++) {
        if (list.Contains(i)) {
            assert((keys.count(i) == 1));
            // std::cout<< list.FindEqual(i)->key << " = key, value = "<< list.FindEqual(i)->value << std::endl;
        } else {
            assert((keys.count(i) == 0));
            // std::cout<< i << " = false" << std::endl;
        }
    }
}

TEST (SkiplistTest, GetValue) {
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
    assert(list.GetValue("abc") == "cba");
    assert(list.GetValue("abcd") == "cba1");
    assert(list.GetValue("abce") == "cba2");
    assert(list.GetValue("abcde") == "cba3");
    std::cout<<list.GetValue("abc")<<std::endl;
    std::cout<<list.GetValue("abcde")<<std::endl;
}

}
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}