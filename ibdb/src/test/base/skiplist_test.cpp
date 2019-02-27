#include <iostream>
#include <set>
#include "base/skiplist.h"
#include "base/arena.h"
#include "port/port.h"


typedef uint64_t Key;
typedef uint32_t Value;

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

void InsertAndLookup() {
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
    for (int i = 0; i < R; i++) {
        if (list.Contains(i)) {
            assert((keys.count(i) == 1));
            std::cout<< i << " = true" << std::endl;
        } else {
            assert((keys.count(i) == 0));
            std::cout<< i << " = false" << std::endl;
        }
    }
}

int main() {
    std::cout<<"skiplist test"<<std::endl;
    ibdb::base::Arena arena;
    Comparator comp;
    // ibdb::base::SkipList<int, Comparator> skip;
    ibdb::base::SkipList<Key, Value, Comparator> skip(comp, &arena);
    assert(!skip.Contains(10));
    std::cout<<skip.Contains(10)<<std::endl;
    InsertAndLookup();
}

