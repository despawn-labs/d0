#include "d0/misc/find_pattern.h"

namespace d0 {

namespace {

constexpr u8 GetNibble(const u8 s) {
  const u8 u = s | 0x20;
  return s >= '0' && s <= '9' ? s - '0' : 10 + (u - 'a');
}

constexpr u8 GetByte(const u8 *s) {
  auto lsn = GetNibble(s[1]);
  auto msn = GetNibble(s[0]);
  return lsn | msn << 4;
}

} // namespace

uptr FindPattern(uptr start, uptr end, const std::string &pattern) {
  // Find the length of the pattern in bytes.
  auto n_pattern = 0;
  {
    auto i = 0;
    while (i < pattern.length()) {
      n_pattern += 1;

      if (pattern[i] == '?')
        i += 2;
      else
        i += 3;
    }
  }

  for (uptr address = start; address <= end - n_pattern; address++) {
    const auto m = reinterpret_cast<u8 *>(address);

    auto pattern_i = 0;
    auto m_i = 0;

    bool failed = false;

    while (pattern_i < pattern.length()) {
      auto m_byte = m[m_i++];

      if (pattern[pattern_i] == '?') {
        pattern_i += 2;
        continue;
      }

      auto p_byte = GetByte(reinterpret_cast<const u8 *>(pattern.data() + pattern_i));

      pattern_i += 3;

      if (m_byte != p_byte) {
        failed = true;
        break;
      }
    }

    if (!failed)
      return address;
  }

  return 0;
}

} // namespace d0