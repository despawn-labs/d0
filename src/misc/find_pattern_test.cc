#include <gtest/gtest.h>

#include <d0/misc/find_pattern.h>

#include <print>

TEST(FindPattern, General) {
  u8 data[] = {0xFF, 0xAB, 0xBB, 0xCC, 0xDD};

  auto start = reinterpret_cast<uptr>(data);
  const auto end = start + sizeof(data);

  auto result = d0::FindPattern(start, end, "AB ? CC DD");
  ASSERT_EQ(start + 1, result);
}