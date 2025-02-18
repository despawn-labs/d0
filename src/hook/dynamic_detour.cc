#include "d0/hook/dynamic_detour.h"

#include "Zydis/Encoder.h"
#include "d0/std/block_allocator.h"
#include "d0/sys/memory.h"

#include <cstdint>
#include <map>
#include <print>

namespace d0 {

constexpr auto k2GB = 0x7FFFFFFF;

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

static uptr AllocateNear(const uptr target, const usize size) {
  static std::map<uptr, BlockAllocator> allocators;

  const auto page_size = GetPageSize();
  const auto start_addr = target - (target % page_size);

  usize offset = 0;
  uptr page_addr = 0;

  while (true) {
    offset += page_size;

    if (offset >= k2GB)
      break;

    const auto fwd_addr = start_addr + offset;
    const auto bck_addr = start_addr - offset;

    if (allocators.contains(fwd_addr)) {
      if (auto &allocator = allocators.at(fwd_addr);
          allocator.CanAllocate(size))
        return allocator.Allocate(size);
    }

    if (allocators.contains(bck_addr)) {
      if (auto &allocator = allocators.at(bck_addr);
          allocator.CanAllocate(size))
        return allocator.Allocate(size);
    }

    const auto fwd_state = GetPageState(fwd_addr);
    const auto bck_state = GetPageState(bck_addr);

    if (fwd_state.is_free) {
      page_addr = fwd_addr;
      break;
    }

    if (bck_state.is_free) {
      page_addr = bck_addr;
      break;
    }
  }

  page_addr = AllocatePage(page_addr);

  PageProtection np{};
  np.SetRead();
  np.SetWrite();
  np.SetExecute();

  PageProtection op{};
  SetPageProtection(page_addr, np, op);

  return 0;
}

static void CreateDetour(uintptr_t target, uintptr_t detour) {
  target = ResolveFunctionAddress(target);
  detour = ResolveFunctionAddress(detour);

  // Create the relay.
  {
    ZydisEncoderRequest r;
    r.mnemonic = ZYDIS_MNEMONIC_JMP;
    r.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
    r.operand_count = 1;
    r.operands[0].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
    r.operands[0].imm.u = detour;

    u8 relay[ZYDIS_MAX_INSTRUCTION_LENGTH];
    usize n_relay;

    if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&r, relay, &n_relay)))
      return;


  }
}

void Test(uintptr_t target) {
  auto page = AllocatePage(0);
  BlockAllocator block_allocator{page, 64, 8};

  FreePage(page);
}

} // namespace d0