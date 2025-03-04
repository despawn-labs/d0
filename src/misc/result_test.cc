#include <gtest/gtest.h>

#include <d0/defs.h>
#include <d0/misc/error.h>
#include <d0/misc/result.h>

/// Test that a [`Result`] maintains a given state.
///
/// This also tests the shorthand constructors, `Ok` and `Err`.
TEST(Result, Stateful) {
  d0::Result<i8, u8> r1{d0::Ok{static_cast<i8>(8)}};

  ASSERT_TRUE(r1.IsOk());
  ASSERT_EQ(r1.Unwrap(), 25);

  d0::Result r2{std::move(r1)};

  ASSERT_TRUE(r2.IsOk());
  ASSERT_EQ(r2.UnwrapError(), -2);
}