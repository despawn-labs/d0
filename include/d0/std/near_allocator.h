#pragma once

#include "d0/std/block_allocator.h"

#include <map>

namespace d0 {

/// Handles managing allocations required to be near a certain address.
///
/// Internally, this class manages a list of page-ranged [`BlockAllocator`]s.
/// These are responsible for performing the actual allocations, while this
/// class is responsible for dispatching calls to the appropriate allocators.
class NearAllocator {
public:
  /// NearAllocator constructor.
  NearAllocator();

  /// NearAllocator destructor.
  ~NearAllocator();

  /// Allocates a given `size` near the address, `near`.
  /// @param near The address to allocate near.
  /// @param size The size to allocate.
  /// @return A pointer to the newly allocated region.
  uptr Allocate(uptr near, usize size);

  /// Frees a given allocation at the given `addr`.
  /// @param addr The address of the allocation to free.
  void Free(uptr addr);

private:
  std::map<uptr, BlockAllocator> allocs_;
};

extern NearAllocator g_near_allocator;

} // namespace d0