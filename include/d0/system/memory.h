///
/// Provides a cross-platform interface for working with
///

#pragma once

#include "d0/defs.h"

#include <result.hpp>

namespace d0 {

enum class D0_API MemoryError {
  kWrite,
  kRead,
};

/// Represents the protection level of a given memory page.
struct D0_API PageProtection {
  static constexpr auto kRead = 1 << 0;
  static constexpr auto kWrite = 1 << 1;
  static constexpr auto kExecute = 1 << 2;

  explicit PageProtection(uint8_t value);
  PageProtection() = default;
  ~PageProtection() = default;

  /// Sets the given flag value.
  template <uint8_t Flag> void Set(bool enabled = true) {
    if (enabled)
      value |= Flag;
    else
      value &= ~Flag;
  }

  /// Sets the "read" flag.
  void SetRead(const bool enabled = true) { Set<kRead>(enabled); }

  /// Sets the "write" flag.
  void SetWrite(const bool enabled = true) { Set<kWrite>(enabled); }

  /// Sets the "execute" flag.
  void SetExecute(const bool enabled = true) { Set<kExecute>(enabled); }

  /// Gets the given flag value.
  template <uint8_t Flag> [[nodiscard]] bool Get() const {
    return (value & Flag) != 0;
  }

  /// Returns the "read" flag.
  [[nodiscard]] bool GetRead() const { return Get<kRead>(); }

  /// Returns the "write" flag.
  [[nodiscard]] bool GetWrite() const { return Get<kWrite>(); }

  /// Returns the "execute" flag.
  [[nodiscard]] bool GetExecute() const { return Get<kExecute>(); }

  void FromPlatform(u32 in);
  [[nodiscard]] u32 ToPlatform() const;

  u8 value;
};

/// Represents the state of a given memory page.
struct PageState {
  PageProtection protection;
  bool is_free;
};

/// Returns the page state for a given address.
/// @param address The address of the page (rounds down).
/// @return The page state.
D0_API PageState GetPageState(uptr address);

/// Returns the page protection for a given address.
/// @param address The address of the page (rounds down).
/// @return The page protection.
D0_API PageProtection GetPageProtection(uptr address);

/// Sets the protection of a given page.
/// @param address The address of the page (rounds down).
/// @param protection The protection to set.
/// @param old_protection The old protection.
D0_API void SetPageProtection(uptr address, const PageProtection &protection,
                              PageProtection &old_protection);

/// Returns the page size.
/// @return The page size.
D0_API usize GetPageSize();

/// Allocates a page. If `address` is not zero, it's used as the page address.
/// @param address The page address, used if not zero.
/// @return The address of the newly allocated page.
D0_API uptr AllocatePage(uptr address);

/// Frees a page at the given address.
/// @param address The address of the page.
D0_API void FreePage(uptr address);

/// Writes to memory at the given address.
/// @param address The address to write to.
/// @param in The buffer of contents to write.
/// @param n_in The length of the buffer.
/// @param n_written The number of bytes written.
D0_API result<void, MemoryError> WriteMemory(uptr address, const u8 *in,
                                             usize n_in, usize &n_written);

/// Reads memory at the given address.
/// @param address The address to read from.
/// @param out The buffer to read to.
/// @param n_out The length of the buffer.
/// @param n_read The number of bytes read.
D0_API result<void, MemoryError> ReadMemory(uptr address, u8 *out, usize n_out,
                                            usize &n_read);

} // namespace d0