#include "d0/detour/dynamic_detour.h"
#include "d0/platform/memory.h"

#include <cstdint>

namespace d0 {

struct State {
  uintptr_t target;
  uintptr_t detour;
  uintptr_t trampoline;
  uintptr_t relay;
};

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

static void *AllocateNear(uintptr_t target) {
  const auto page_size = GetPageSize();
  auto start_addr = (target / page_size) * page_size;

  size_t offset = page_size;

  while (true) {


    offset += page_size;
  }
}

static void CreateDetour(uintptr_t target, uintptr_t detour) {
  target = ResolveFunctionAddress(target);
}

}