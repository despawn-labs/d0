#pragma once

#include <cstdint>

namespace d0 {

struct Protection {
  static constexpr auto kRead = 1 << 0;
  static constexpr auto kWrite = 1 << 1;
  static constexpr auto kExecute = 1 << 2;

  Protection() = default;
  ~Protection() = default;

  inline void Read(bool enabled = true) {

  }

  inline void Write(bool enabled = true) {

  }

  inline void Execute(bool enabled = true) {
    if (enabled)
      value |= kExecute;
  }

  uint8_t value;
};

size_t GetPageSize();

}