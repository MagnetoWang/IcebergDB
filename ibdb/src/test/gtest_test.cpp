#include <iostream>
#include "base/arena.h"
#include "base/atomic_pointer.h"
#include "base/random.h"
#include "gtest/gtest.h"

namespace ibdb {
namespace base {
class BaseTest : public ::testing::Test{

};

TEST(BaseTest, Simple) {
    std::vector<std::pair<size_t, char*>> allocated;
    ibdb::base::Arena arena;
    const int N = 10000;
    size_t bytes = 0;
    ibdb::base::Random rnd(301);

    for (int i = 0; i < N; i++) {
    size_t s;
    if (i % (N / 10) == 0) {
        s = i;
    } else {
        s = rnd.OneIn(4000) ? rnd.Uniform(6000) :
            (rnd.OneIn(10) ? rnd.Uniform(100) : rnd.Uniform(20));
    }
    if (s == 0) {
        // Our arena disallows size 0 allocations.
        s = 1;
    }
    char* r;
    if (rnd.OneIn(10)) {
        r = arena.AllocateAligned(s);
    } else {
        r = arena.Allocate(s);
    }
    for (size_t b = 0; b < s; b++) {
        // Fill the "i"th allocation with a known bit pattern
        r[b] = i % 256;
    }
    bytes += s;
    allocated.push_back(std::make_pair(s, r));
    }
}

}
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}