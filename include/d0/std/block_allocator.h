#pragma once

#include "d0/defs.h"
#include "d0/std/bit_map.h"

#include <vector>

namespace d0 {

class BlockAllocator {
public:
  explicit BlockAllocator(uptr addr, usize size, usize block_size);
  ~BlockAllocator();

  uptr Allocate(usize size);

  void Free(uptr addr);

  bool CanAllocate(usize size) const;

  usize GetAllocationCount() const;

private:
  [[nodiscard]] usize FindSmallestBlock(usize size) const;
  bool FindFreeBlock(usize block_size, usize &index) const;

  void MarkUsedBlock(usize index, usize n_units) const;
  void MarkFreeBlock(usize index, usize n_units) const;

private:
  uptr region_addr_;
  usize region_size_;

  usize unit_size_;

  BitMap map_;
  std::vector<usize> allocations_;
};

} // namespace d0