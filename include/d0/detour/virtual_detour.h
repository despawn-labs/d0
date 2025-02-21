#pragma once

#include "d0/defs.h"
#include "d0/detour/detour.h"

namespace d0 {

class VirtualDetourBase : public Detour {
public:
  VirtualDetourBase(uptr target, usize index, uptr detour);
  ~VirtualDetourBase() override;

  /// Initializes the detour by creating the trampoline and relay code.
  /// This must be called prior to using `Enable`/`Disable`.
  void Initialize() override;

  /// Shuts down the detour. This must be called before the destructor is called.
  void Shutdown() override;

  /// Enables the detour.
  /// This function can only be called after `Initialize` and before `Shutdown`.
  void Enable() override;

  /// Disables the detour.
  /// This function can only be called after `Initialize` and before `Shutdown`.
  void Disable() override;

private:
  uptr target_;
  uptr detour_;

  uptr *slot_;
};

} // namespace d0