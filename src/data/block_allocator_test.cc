#include <gtest/gtest.h>

#include <d0/data/block_allocator.h>
#include <d0/system/memory.h>

using namespace d0;

TEST(BlockAllocator, Allocate) {
  const auto region = AllocatePage(0);
  BlockAllocator alloc{region, GetPageSize(), 8};
  alloc.Allocate(8);

  FreePage(region);
}