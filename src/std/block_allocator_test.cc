
#include <gtest/gtest.h>

#include <d0/std/block_allocator.h>
#include <d0/sys/memory.h>
using namespace d0;

TEST(BlockAllocator, Allocate) {
  const auto region = AllocatePage(0);
  BlockAllocator alloc{region, GetPageSize(), 8};
  alloc.Allocate(8);

  FreePage(region);
}