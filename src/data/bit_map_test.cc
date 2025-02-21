#include <gtest/gtest.h>

#include <d0/data/bit_map.h>

using namespace d0;

TEST(BitMap, Overview) {
  BitMap bm{32};

  ASSERT_FALSE(bm.Get(0));

  bm.Set(0, true);
  ASSERT_TRUE(bm.Get(0));

  bm.Set(1, true);
  bm.Set(2, true);

  ASSERT_TRUE(bm.Get(0));
  ASSERT_TRUE(bm.Get(1));
  ASSERT_TRUE(bm.Get(2));

  bm.Set(1, false);

  ASSERT_TRUE(bm.Get(0));
  ASSERT_FALSE(bm.Get(1));
  ASSERT_TRUE(bm.Get(2));
}