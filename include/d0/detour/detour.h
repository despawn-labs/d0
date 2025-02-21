#pragma once

#include <memory>

#include "d0/std/near_allocator.h"

namespace d0::detour {

/// The base pure-virtual detour class.
class Detour {
public:
  virtual ~Detour() = default;

  /// Initializes the detour by creating the trampoline and relay code.
  /// This must be called prior to using `Enable`/`Disable`.
  virtual void Initialize() = 0;

  /// Shuts down the detour. This must be called before the destructor is
  /// called.
  virtual void Shutdown() = 0;

  /// Enables the detour.
  /// This function can only be called after `Initialize` and before `Shutdown`.
  virtual void Enable() = 0;

  /// Disables the detour.
  /// This function can only be called after `Initialize` and before `Shutdown`.
  virtual void Disable() = 0;
};

} // namespace d0::detour