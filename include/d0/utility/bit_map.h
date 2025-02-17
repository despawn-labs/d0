#pragma once

#include "d0/defs.h"

namespace d0 {

class BitMap {
public:
  BitMap(usize size);
  ~BitMap();

  BitMap(const BitMap &other);
  BitMap(BitMap &&other) noexcept;

  void Resize(usize size);

  void Set(usize index, bool value) const;
  bool Get(usize index) const;

private:
  usize size_;
  usize n_data_;
  usize *data_;
};

} // namespace d0