#include "d0/std/near_allocator.h"

#include "d0/std/runtime_exception.h"
#include "d0/sys/memory.h"

#include <print>

namespace d0 {

constexpr auto k2GB = 0x7FFFFFFF;
constexpr auto kNearAllocatorBlockSize = 8;

NearAllocator::NearAllocator() {}

NearAllocator::~NearAllocator() = default;

uptr NearAllocator::Allocate(const uptr near, const usize size) {
  // Validate the input.
  if (size <= 0)
    throw RuntimeException("Allocation size must be greater than zero.");

  // Search for a free page near the given address.
  const auto page_size = sys::GetPageSize();

  const auto start_addr = near - (near % page_size);
  auto i = 1;

  uptr page_addr = 0;

  while (true) {
    const auto fwd_addr = start_addr + i * page_size;
    const auto bck_addr = start_addr - i * page_size;

    // Max out at 2GB and return nullptr.
    if (fwd_addr - start_addr >= k2GB)
      return 0;

    const auto fwd_state = sys::GetPageState(fwd_addr);
    const auto bck_state = sys::GetPageState(bck_addr);

    // Check if an allocator already exists for this page.
    std::shared_ptr<BlockAllocator> allocator{nullptr};

    // Check if the given allocator is available.
    if (allocators_.contains(fwd_addr))
      if (const auto fwd_allocator = allocators_.at(fwd_addr);
          fwd_allocator->CanAllocate(size))
        allocator = fwd_allocator;
      else if (allocators_.contains(bck_addr))
        if (const auto bck_allocator = allocators_.at(fwd_addr);
            bck_allocator->CanAllocate(size))
          allocator = bck_allocator;

    // If we have an allocator, we'll simply return immediately using it.
    if (allocator)
      return allocator->Allocate(size);

    // Check if the forwards page is free.
    if (fwd_state.is_free) {
      page_addr = fwd_addr;
      break;
    }

    // Check if the backwards page is free.
    if (bck_state.is_free) {
      page_addr = bck_addr;
      break;
    }

    i += 1;
  }

  // We couldn't find a page, fail.
  if (page_addr == 0)
    return 0;

  // Allocate the page.
  if (!sys::AllocatePage(page_addr))
    throw RuntimeException("Failed to allocate page at: 0x{:X}", page_addr);

  // Create a new allocator for the page.
  const auto allocator = std::make_shared<BlockAllocator>(
      page_addr, sys::GetPageSize(), kNearAllocatorBlockSize);

  // Insert the allocator.
  allocators_[page_addr] = allocator;

  // Create the new allocation.
  return allocator->Allocate(size);
}

void NearAllocator::Free(const uptr addr) {
  const auto page_size = sys::GetPageSize();
  const auto page_addr = addr - addr % page_size;

  if (!allocators_.contains(page_addr))
    throw RuntimeException("No allocator found for the given address.");

  const auto &allocator = allocators_.at(page_addr);
  allocator->Free(addr);

  if (allocator->GetAllocationCount() == 0) {
    sys::FreePage(page_addr);
    allocators_.erase(page_addr);
  }
}

static NearAllocator near_allocator;

uptr AllocateNear(const uptr near, const usize size) {
  return near_allocator.Allocate(near, size);
}

void FreeNear(const uptr addr) { return near_allocator.Free(addr); }

} // namespace d0