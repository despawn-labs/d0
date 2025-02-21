#include "d0/data/block_allocator.h"
#include "d0/misc/runtime_exception.h"

namespace d0 {

BlockAllocator::BlockAllocator(const uptr addr, const usize size,
                               const usize block_size)
    : region_addr_{addr}, region_size_{size}, unit_size_{block_size}, map_{0} {
  if (size % unit_size_ != 0)
    throw RuntimeException("Region size must be multiple of block size.");

  map_.Resize(size / unit_size_);

  allocations_.resize(size / unit_size_);
  std::fill(allocations_.begin(), allocations_.end(), 0);
}

BlockAllocator::~BlockAllocator() = default;

uptr BlockAllocator::Allocate(const usize size) {
  // Check the size of the allocation.
  if (size == 0)
    throw RuntimeException("Allocation size must be greater than zero.");

  if (size > region_size_)
    throw RuntimeException("Allocation size is too big for allocator.");

  // Find the nearest containing block size.
  auto block_size = FindSmallestBlock(size);

  // Find a free block.
  usize block_index = 0;
  if (!FindFreeBlock(block_size, block_index))
    throw RuntimeException("Failed to find free block of size, {}.",
                           block_size);

  // Mark the block as used.
  MarkUsedBlock(block_index, block_size / unit_size_);

  // Store the allocation.
  allocations_[block_index] = block_size;

  return region_addr_ + block_index * unit_size_;
}

void BlockAllocator::Free(const uptr addr) {
  // Ensure the address is within our managed region.
  if (addr < region_addr_ || addr >= region_addr_ + region_size_)
    throw RuntimeException("Address out of bounds of allocator region.");

  // Ensure the relative address is a multiple of the unit size.
  if ((addr - region_addr_) % unit_size_ != 0)
    throw RuntimeException(
        "Address should be a multiple of the allocator's unit size.");

  const auto block_index = (addr - region_addr_) / unit_size_;
  const auto block_size = allocations_[block_index];

  // Check that an allocation exists.
  if (block_size == 0)
    throw RuntimeException("No allocation found for the given address.");

  // Free the block.
  MarkFreeBlock(block_index, block_size / unit_size_);
  allocations_[block_index] = 0;
}

bool BlockAllocator::CanAllocate(const usize size) const {
  // Check the size of the allocation.
  if (size == 0)
    throw RuntimeException("Allocation size must be greater than zero.");

  if (size > region_size_)
    throw RuntimeException("Allocation size is too big for allocator.");

  // Find the nearest containing block size.
  const auto block_size = FindSmallestBlock(size);

  // Find a free block.
  usize block_index = 0;
  return FindFreeBlock(block_size, block_index);
}

usize BlockAllocator::GetAllocationCount() const {
  auto c = 0;
  for (const auto &size : allocations_)
    c += size != 0 ? 1 : 0;
  return c;
}

usize BlockAllocator::FindSmallestBlock(const usize size) const {
  usize n = 1;

  while (true) {
    const auto block_size = unit_size_ * n++;
    if (size > block_size)
      continue;

    return block_size;
  }

  return 0;
}

bool BlockAllocator::FindFreeBlock(const usize block_size, usize &index) const {
  const auto units_per_region = region_size_ / unit_size_;
  const auto units_per_block = block_size / unit_size_;

  // Loop through in "units per block" chunks.
  for (usize i = 0; i < units_per_region; i += units_per_block) {
    bool is_free = true;

    // Loop through the individual bits in the chunks.
    for (usize j = i; j < i + units_per_block; j++) {
      // Check if part of this region is allocated.
      if (map_.Get(j)) {
        is_free = false;
        break;
      }
    }

    // If it was free, we'll return true and set it's index.
    if (is_free) {
      index = i;
      return true;
    }
  }

  return false;
}

void BlockAllocator::MarkUsedBlock(const usize index,
                                   const usize n_units) const {
  for (auto i = index; i < n_units; i++)
    map_.Set(i, true);
}

void BlockAllocator::MarkFreeBlock(const usize index,
                                   const usize n_units) const {
  for (auto i = index; i < n_units; i++)
    map_.Set(i, false);
}

} // namespace d0