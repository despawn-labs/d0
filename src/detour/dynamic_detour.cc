#include "d0/detour/dynamic_detour.h"
#include "d0/platform/memory.h"
#include "d0/utility/bit_map.h"

#include <cstdint>
#include <map>
#include <print>

namespace d0 {

constexpr auto k2GB = 0x7FFFFFFF;

class BlockAllocator {
public:
  explicit BlockAllocator(uptr addr, usize size);
  ~BlockAllocator();

  uptr Allocate(usize size);

  void Free(uptr addr);

private:
  uptr addr_;
  usize size_;

  BitMap bit_map_;
};

class DynamicDetourAllocator {
public:
  DynamicDetourAllocator();
  ~DynamicDetourAllocator();

  uptr Allocate(uptr for_addr, usize size);

  void Free(uptr addr);

private:
};

BlockAllocator::BlockAllocator(uptr addr, usize size)
    : addr_{addr}, size_{size}, bit_map_{} {}

BlockAllocator::~BlockAllocator() {}

uptr BlockAllocator::Allocate(usize size) {}

void BlockAllocator::Free(uptr addr) {}

DynamicDetourAllocator::DynamicDetourAllocator() {}

DynamicDetourAllocator::~DynamicDetourAllocator() {}

uptr DynamicDetourAllocator::Allocate(uptr for_addr, usize size) {}

void DynamicDetourAllocator::Free(uptr addr) {}

static uintptr_t ResolveFunctionAddress(uintptr_t address) {
  auto data = reinterpret_cast<uint8_t *>(address);

  if (data[0] == 0x48 && data[1] == 0xFF && data[2] == 0x25) {
    auto offset = *reinterpret_cast<int32_t *>(address + 3);
    return ResolveFunctionAddress(address + 7 + offset);
  }

  if (data[0] == 0xFF && data[1] == 0x25) {
    auto offset = *reinterpret_cast<int32_t *>(address + 2);
    return ResolveFunctionAddress(address + 6 + offset);
  }

  return address;
}

static uptr AllocateNear(uptr target) {
  const auto page_size = GetPageSize();
  auto start_addr = target - (target % page_size);

  std::println("start=0x{:X} - page_size=0x{:X}", start_addr, page_size);
  usize offset = 0;

  uptr page_addr = 0;
  PageState page_state{};

  while (true) {
    offset += page_size;

    if (offset >= k2GB)
      break;

    auto fwd_addr = start_addr + offset;
    auto bck_addr = start_addr - offset;

    auto fwd_state = GetPageState(fwd_addr);
    auto bck_state = GetPageState(bck_addr);

    std::println("(+) 0x:{:X}: is_free={}", fwd_addr, fwd_state.is_free);
    std::println("(-) 0x:{:X}: is_free={}", bck_addr, bck_state.is_free);

    if (fwd_state.is_free) {
      page_addr = fwd_addr;
      page_state = fwd_state;
      break;
    }

    if (bck_state.is_free) {
      page_addr = bck_addr;
      page_state = bck_state;
      break;
    }
  }

  page_addr = AllocatePage(page_addr);

  PageProtection new_prot;
  new_prot.SetRead();
  new_prot.SetWrite();
  new_prot.SetExecute();

  PageProtection old_prot{};
  SetPageProtection(page_addr, new_prot, old_prot);

  return page_addr;
}

static void CreateDetour(uintptr_t target, uintptr_t detour) {
  target = ResolveFunctionAddress(target);
}

void Test(uintptr_t target) { CreateDetour(target, 0); }

} // namespace d0