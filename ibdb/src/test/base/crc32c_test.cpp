#include "base/crc32c.h"
#include "gtest/gtest.h"
#include <assert.h>
namespace ibdb {
namespace base {
namespace crc32c {

class CRC { };

TEST(CRC, StandardResults) {
  // From rfc3720 section B.4.
  char buf[32];

  memset(buf, 0, sizeof(buf));
  assert(0x8a9136aa == Value(buf, sizeof(buf)));

  memset(buf, 0xff, sizeof(buf));
  assert(0x62a8ab43 == Value(buf, sizeof(buf)));

  for (int i = 0; i < 32; i++) {
    buf[i] = i;
  }
  assert(0x46dd794e == Value(buf, sizeof(buf)));

  for (int i = 0; i < 32; i++) {
    buf[i] = 31 - i;
  }
  assert(0x113fdb5c == Value(buf, sizeof(buf)));

  unsigned char data[48] = {
    0x01, 0xc0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x14, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x04, 0x00,
    0x00, 0x00, 0x00, 0x14,
    0x00, 0x00, 0x00, 0x18,
    0x28, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
  };
  assert(0xd9963a56 == Value(reinterpret_cast<char*>(data), sizeof(data)));
}

TEST(CRC, Values) {
//   ASSERT_NE(Value("a", 1), Value("foo", 3));
}

TEST(CRC, Extend) {
  assert(Value("hello world", 11) ==
            Extend(Value("hello ", 6), "world", 5));
}

TEST(CRC, Mask) {
  uint32_t crc = Value("foo", 3);
  assert(crc != Mask(crc));
  assert(crc != Mask(Mask(crc)));
  assert(crc == Unmask(Mask(crc)));
  assert(crc == Unmask(Unmask(Mask(Mask(crc)))));
}

} // crc32c
} // base
} // ibdb

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}