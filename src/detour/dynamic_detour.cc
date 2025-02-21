#include "d0/detour/dynamic_detour.h"
#include "d0/data/near_allocator.h"
#include "d0/misc/runtime_exception.h"
#include "d0/system/memory.h"

#include <cstdint>
#include <list>
#include <map>
#include <print>

#include <Zydis/Zydis.h>

namespace d0 {

constexpr auto k2GB = 0x7FFFFFFF;

struct StolenInstruction {
  ZydisDecodedInstruction instruction;
  ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

  usize rva;
  usize length;

  u8 *data;
};

static uintptr_t ResolveFunctionAddress(const uintptr_t address) {
  const auto data = reinterpret_cast<uint8_t *>(address);

  if (data[0] == 0xE9) {
    const auto offset = *reinterpret_cast<int32_t *>(address + 1);
    return ResolveFunctionAddress(address + 5 + offset);
  }

  if (data[0] == 0x48 && data[1] == 0xFF && data[2] == 0x25) {
    const auto offset = *reinterpret_cast<int32_t *>(address + 3);
    return ResolveFunctionAddress(address + 7 + offset);
  }

  if (data[0] == 0xFF && data[1] == 0x25) {
    const auto offset = *reinterpret_cast<int32_t *>(address + 2);
    return ResolveFunctionAddress(address + 6 + offset);
  }

  return address;
}

static bool IsRIPRelative(const StolenInstruction &inst, usize &offset,
                          usize &size) {
  for (auto i = 0; i < inst.instruction.operand_count; i++) {
    const auto &operand = inst.operands[i];

    if (operand.type == ZYDIS_OPERAND_TYPE_MEMORY &&
        operand.mem.base == ZYDIS_REGISTER_RIP) {
      offset = operand.mem.disp.offset;
      size = operand.mem.disp.size;

      return true;
    }
  }

  return false;
}

static bool IsRelativeCall(const StolenInstruction &inst) {
  return inst.instruction.mnemonic == ZYDIS_MNEMONIC_CALL and
         inst.data[0] == 0xE8;
}

static bool IsRelativeJump(const StolenInstruction &inst) {
  bool is_any_jmp = inst.instruction.mnemonic >= ZYDIS_MNEMONIC_JB &&
                    inst.instruction.mnemonic <= ZYDIS_MNEMONIC_JZ;
  bool is_jmp = inst.instruction.mnemonic == ZYDIS_MNEMONIC_JMP;
  bool starts_with_eb_or_e9 = inst.data[0] == 0xEB || inst.data[0] == 0xE9;
  return is_jmp ? starts_with_eb_or_e9 : is_any_jmp;
}

static void EncodeAbsoluteJump64(std::vector<u8> &dest, uptr target) {
  ZydisEncoderRequest r = {};

  r.mnemonic = ZYDIS_MNEMONIC_MOV;
  r.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
  r.operand_count = 2;
  r.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
  r.operands[0].reg.value = ZYDIS_REGISTER_R10;
  r.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
  r.operands[1].imm.u = target;

  u8 mov[ZYDIS_MAX_INSTRUCTION_LENGTH];
  usize n_mov = sizeof(mov);

  if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&r, mov, &n_mov)))
    throw RuntimeException("Failed to encode MOV instruction.");

  r.mnemonic = ZYDIS_MNEMONIC_JMP;
  r.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
  r.operand_count = 1;
  r.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
  r.operands[0].reg.value = ZYDIS_REGISTER_R10;

  u8 jmp[ZYDIS_MAX_INSTRUCTION_LENGTH];
  usize n_jmp = sizeof(jmp);

  if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&r, jmp, &n_jmp)))
    throw RuntimeException("Failed to encode JMP instruction.");

  dest.insert(dest.end(), &mov[0], &mov[n_mov]);
  dest.insert(dest.end(), &jmp[0], &jmp[n_jmp]);
}

static void EncodeRelativeJump(std::vector<u8> &dest, i32 disp) {
  ZydisEncoderRequest r = {};

  r.mnemonic = ZYDIS_MNEMONIC_JMP;
  r.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
  r.operand_count = 1;
  r.operands[0].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
  r.operands[0].imm.s = disp;

  u8 jmp[ZYDIS_MAX_INSTRUCTION_LENGTH];
  usize n_jmp = sizeof(jmp);

  if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&r, jmp, &n_jmp)))
    throw RuntimeException("Failed to encode JMP instruction.");

  dest.insert(dest.end(), &jmp[0], &jmp[n_jmp]);
}

DynamicDetourBase::DynamicDetourBase(
    std::shared_ptr<NearAllocator> near_allocator, const uptr target,
    const uptr detour)
    : near_allocator_(near_allocator), state_{State::kDormant},
      target_{ResolveFunctionAddress(target)},
      detour_{ResolveFunctionAddress(detour)} {}

DynamicDetourBase::~DynamicDetourBase() {}

void DynamicDetourBase::Initialize() {
  if (state_ != State::kDormant)
    return;

  _Setup();

  state_ = State::kDisabled;
}

void DynamicDetourBase::Shutdown() {
  if (state_ == State::kDormant)
    return;

  if (relay_ != 0)
    near_allocator_->Free(relay_);

  if (trampoline_ != 0)
    near_allocator_->Free(trampoline_);

  state_ = State::kDormant;
}

void DynamicDetourBase::Enable() {
  if (state_ == State::kEnabled || state_ == State::kDormant)
    return;

  std::vector<u8> jmp;
  jmp.reserve(5);

  EncodeRelativeJump(jmp, static_cast<i32>(relay_ - (target_ + 5)));

  PageProtection rwe{};
  rwe.SetRead();
  rwe.SetWrite();
  rwe.SetExecute();

  PageProtection old_p_1{};

  const auto page_size = GetPageSize();
  const auto page = target_ - (target_ % page_size);
  SetPageProtection(page, rwe, old_p_1);

  memcpy(reinterpret_cast<u8 *>(target_), jmp.data(), jmp.size());

  PageProtection old_p_2{};
  SetPageProtection(page, old_p_1, old_p_2);

  state_ = State::kEnabled;
}

void DynamicDetourBase::Disable() {
  if (state_ == State::kDisabled || state_ == State::kDormant)
    return;

  PageProtection rwe{};
  rwe.SetRead();
  rwe.SetWrite();
  rwe.SetExecute();

  PageProtection old_p;

  const auto page_size = GetPageSize();
  const auto page = target_ - (target_ % page_size);
  SetPageProtection(page, rwe, old_p);

  memcpy(reinterpret_cast<u8 *>(target_), instructions_.data(),
         instructions_.size());

  PageProtection old_p_2;
  SetPageProtection(page, old_p, old_p_2);

  state_ = State::kDisabled;
}

uptr DynamicDetourBase::GetTrampoline() const { return trampoline_; }

static void DisassembleRegion(uptr addr, usize size) {
  ZydisDecoder decoder;
  ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);

  ZydisFormatter formatter;
  ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

  ZydisDecodedInstruction instruction;
  ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];

  usize offset = 0;

  while (ZYAN_SUCCESS(
      ZydisDecoderDecodeFull(&decoder, reinterpret_cast<void *>(addr + offset),
                             size - offset, &instruction, operands))) {
    std::print("{:0X}: ", addr + offset);

    char buffer[256];
    ZydisFormatterFormatInstruction(&formatter, &instruction, operands,
                                    instruction.operand_count_visible, buffer,
                                    sizeof(buffer), addr + offset, ZYAN_NULL);

    std::println("{}", buffer);

    offset += instruction.length;
  }
}

void DynamicDetourBase::_Setup() {
  // Determine the instructions we need to steal.
  usize stolen_instructions_length = 0;
  usize n_jmps_or_calls = 0;

  std::list<StolenInstruction> stolen_instructions;
  {
    ZydisDecoder decoder;
    ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64,
                     ZYDIS_STACK_WIDTH_64);

    ZydisFormatter formatter;
    ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    StolenInstruction stolen_instruction{};

    usize offset = 0;

    while (ZYAN_SUCCESS(ZydisDecoderDecodeFull(
        &decoder, reinterpret_cast<void *>(target_ + offset),
        ZYDIS_MAX_INSTRUCTION_LENGTH, &stolen_instruction.instruction,
        stolen_instruction.operands))) {
#if 0
      std::print("{:0X}: ", detour_ + offset);

      char buffer[256];
      ZydisFormatterFormatInstruction(
          &formatter, &instruction, operands, instruction.operand_count_visible,
          buffer, sizeof(buffer), target_ + offset, ZYAN_NULL);

      std::println("{}", buffer);
#endif

      stolen_instruction.rva = offset;
      stolen_instruction.data = reinterpret_cast<u8 *>(target_ + offset);
      stolen_instruction.length = stolen_instruction.instruction.length;

      if (IsRelativeCall(stolen_instruction) ||
          IsRelativeJump(stolen_instruction))
        n_jmps_or_calls += 1;

      stolen_instructions.push_back(stolen_instruction);

      stolen_instructions_length += stolen_instruction.length;

      offset += stolen_instruction.instruction.length;
      if (offset >= 5)
        break;
    }

    instructions_.clear();
    instructions_.resize(stolen_instructions_length);
    memcpy(instructions_.data(), reinterpret_cast<void *>(target_),
           stolen_instructions_length);
  }

  // Create the relay.
  {
    std::vector<u8> relay;
    EncodeAbsoluteJump64(relay, detour_);

    n_relay_ = relay.size();
    relay_ = near_allocator_->Allocate(target_, n_relay_);
    memcpy(reinterpret_cast<u8 *>(relay_), relay.data(), relay.size());
  }

  // Create the trampoline.
  {
    n_trampoline_ = stolen_instructions_length + n_jmps_or_calls * 13 + 5;
    trampoline_ = near_allocator_->Allocate(target_, n_trampoline_);

    auto ait = trampoline_ + stolen_instructions_length + 5;
    usize ait_offset = 0;

    memcpy(reinterpret_cast<u8 *>(trampoline_), reinterpret_cast<u8 *>(target_),
           stolen_instructions_length);

    // Fix up stolen instructions.
    for (auto &stolen_instruction : stolen_instructions) {
      // Fix up RIP-relative memory addressing.
      usize rip_offset = 0;
      usize rip_size = 0;

      if (IsRIPRelative(stolen_instruction, rip_offset, rip_size)) {
        switch (rip_size) {
        case 1: {
          auto disp = *reinterpret_cast<i8 *>(
              trampoline_ + stolen_instruction.rva + rip_offset);
          disp -= trampoline_ - target_;
          memcpy(reinterpret_cast<u8 *>(trampoline_ + stolen_instruction.rva +
                                        rip_offset),
                 &disp, 1);
          break;
        }

        case 2: {
          auto disp = *reinterpret_cast<i16 *>(
              trampoline_ + stolen_instruction.rva + rip_offset);
          disp -= trampoline_ - target_;
          memcpy(reinterpret_cast<u8 *>(trampoline_ + stolen_instruction.rva +
                                        rip_offset),
                 &disp, 1);
          break;
        }

        case 4: {
          auto disp = *reinterpret_cast<i32 *>(
              trampoline_ + stolen_instruction.rva + rip_offset);
          disp -= trampoline_ - target_;
          memcpy(reinterpret_cast<u8 *>(trampoline_ + stolen_instruction.rva +
                                        rip_offset),
                 &disp, 1);
          break;
        }
        default:
          break;
        }
      } else if (IsRelativeJump(stolen_instruction)) {
        // Add a redirection in the jump table.
        std::vector<u8> abs_jmp{};
        abs_jmp.reserve(13);

        isize disp = 0;

        for (auto i = 0; i < stolen_instruction.instruction.operand_count;
             i++) {
          const auto &operand = stolen_instruction.operands[i];
          if (operand.type != ZYDIS_OPERAND_TYPE_IMMEDIATE)
            continue;
          disp = operand.imm.value.s;
          break;
        }

        const auto target =
            target_ + stolen_instruction.rva + stolen_instruction.length + disp;

        EncodeAbsoluteJump64(abs_jmp, target);

        memcpy(reinterpret_cast<u8 *>(ait + ait_offset), abs_jmp.data(),
               abs_jmp.size());

        ait_offset += abs_jmp.size();
      } else if (IsRelativeCall(stolen_instruction)) {
        // TODO
      }
    }

    // Add the jump back to the original function.
    std::vector<u8> jump_back;
    jump_back.reserve(5);

    const auto disp = (target_ + stolen_instructions_length) -
                      (trampoline_ + stolen_instructions_length + 5);

    EncodeRelativeJump(jump_back, disp);

    memcpy(reinterpret_cast<u8 *>(trampoline_ + stolen_instructions_length),
           jump_back.data(), jump_back.size());
  }
}

} // namespace d0