#include "d0/detour/virtual_detour.h"

namespace d0 {

VirtualDetourBase::VirtualDetourBase(uptr target, usize index, uptr detour)
    : target_(0), detour_(detour), slot_(nullptr) {
  auto vmt = *reinterpret_cast<uptr **>(target);
  slot_ = &vmt[index];
  target_ = *slot_;
}

VirtualDetourBase::~VirtualDetourBase() {}

void VirtualDetourBase::Initialize() {}

void VirtualDetourBase::Shutdown() {}

void VirtualDetourBase::Enable() {
  *slot_ = detour_;
}

void VirtualDetourBase::Disable() {
  *slot_ = target_;
}

} // namespace d0::detour