#pragma once

#include "d0/defs.h"
#include "d0/detour/detour.h"

#include <vector>

namespace d0 {

/// Base implementation of a trampoline-based detour.
class D0_API DynamicDetourBase : public Detour {
public:
  /// The state of the detour.
  enum class State { kDormant = 0, kEnabled = 1, kDisabled = 2 };

public:
  /// DynamicDetourBase constructor.
  /// @param near_allocator The allocator to use.
  /// @param target The target function to detour.
  /// @param detour The "detour" function.
  DynamicDetourBase(std::shared_ptr<NearAllocator> near_allocator, uptr target,
                    uptr detour);

  /// DynamicDetourBase destructor.
  ~DynamicDetourBase() override;

  /// Initializes the detour by creating the trampoline and relay code.
  /// This must be called prior to using `Enable`/`Disable`.
  void Initialize() override;

  /// Shuts down the detour. This must be called before the destructor is
  /// called.
  void Shutdown() override;

  /// Enables the detour by placing a jump instruction to the trampoline in its
  /// prologue. This function can only be called after `Initialize` and before
  /// `Shutdown`.
  void Enable() override;

  /// Disables the detour by resetting the original instructions.
  /// This function can only be called after `Initialize` and before `Shutdown`.
  void Disable() override;

  /// Returns the address to the trampoline function.
  /// @return The address of the trampoline function.
  [[nodiscard]] uptr GetTrampoline() const;

private:
  void _Setup();

private:
  std::shared_ptr<NearAllocator> near_allocator_;

  State state_;

  uptr target_;
  uptr detour_;

  uptr relay_;
  usize n_relay_;

  uptr trampoline_;
  usize n_trampoline_;

  std::vector<u8> instructions_;
};

/// Template implementation of a trampoline-based detour.
/// @tparam Return The return type of the target function.
/// @tparam Args The argument types of the target function.
template <typename Return, typename... Args>
class DynamicDetour final : public DynamicDetourBase {
public:
  /// The function pointer type of the target and detour functions.
  using Fn = Return (*)(Args...);

public:
  /// DynamicDetour constructor.
  /// @param near_allocator The allocator to use.
  /// @param target The target function to detour.
  /// @param detour The "detour" function.
  DynamicDetour(const std::shared_ptr<NearAllocator> &near_allocator, Fn target,
                Fn detour)
      : DynamicDetourBase(near_allocator, reinterpret_cast<uptr>(target),
                          reinterpret_cast<uptr>(detour)) {}

  /// DynamicDetour destructor.
  ~DynamicDetour() override = default;

  /// Calls the original function via the trampoline.
  /// @param args The arguments of the target function.
  /// @return The return value of the target function.
  Return Call(Args... args) const {
    return reinterpret_cast<Fn>(GetTrampoline())(args...);
  }
};

} // namespace d0