#include <iostream>
#include <set>

#include "base/skiplist.h"
#include "base/arena.h"
#include "base/logging.h"


#include "port/port.h"
#include "gtest/gtest.h"

namespace ibdb {
namespace base {

class Logging { };

TEST(Logging, NumberToString) {
//   ASSERT_EQ("0", NumberToString(0));
//   ASSERT_EQ("1", NumberToString(1));
//   ASSERT_EQ("9", NumberToString(9));

//   ASSERT_EQ("10", NumberToString(10));
//   ASSERT_EQ("11", NumberToString(11));
//   ASSERT_EQ("19", NumberToString(19));
//   ASSERT_EQ("99", NumberToString(99));

//   ASSERT_EQ("100", NumberToString(100));
//   ASSERT_EQ("109", NumberToString(109));
//   ASSERT_EQ("190", NumberToString(190));
//   ASSERT_EQ("123", NumberToString(123));
//   ASSERT_EQ("12345678", NumberToString(12345678));
//   static_assert(std::numeric_limits<uint64_t>::max() == 18446744073709551615U,
//                 "Test consistency check");
//   ASSERT_EQ("18446744073709551000", NumberToString(18446744073709551000U));
//   ASSERT_EQ("18446744073709551600", NumberToString(18446744073709551600U));
//   ASSERT_EQ("18446744073709551610", NumberToString(18446744073709551610U));
//   ASSERT_EQ("18446744073709551614", NumberToString(18446744073709551614U));
//   ASSERT_EQ("18446744073709551615", NumberToString(18446744073709551615U));
    assert("0"  == NumberToString(0));
    assert("11" == NumberToString(11));
    assert("100" == NumberToString(100));
    assert("12345678" == NumberToString(12345678));
    assert("18446744073709551000" == NumberToString(18446744073709551000U));
    assert("18446744073709551615"== NumberToString(18446744073709551615U));
    // std::cout<<NumberToString(0)<<std::endl;
}

} // base
} // ibdb

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}