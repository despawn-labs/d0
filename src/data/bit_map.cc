#include "d0/data/bit_map.h"
#include "d0/misc/runtime_exception.h"

#include <memory>

namespace d0 {

constexpr auto kBitMapDataBytes = sizeof(usize);
constexpr auto kBitMapDataBits = kBitMapDataBytes * 8;

BitMap::BitMap(const usize size) : size_{0}, n_data_{0}, data_{nullptr} {
  Resize(size);
}

BitMap::~BitMap() = default;

BitMap::BitMap(const BitMap &other) {
  size_ = other.size_;
  n_data_ = other.n_data_;

  data_ = new usize[n_data_];
  memcpy(data_, other.data_, n_data_);
}

BitMap::BitMap(BitMap &&other) noexcept {
  size_ = other.size_;
  n_data_ = other.n_data_;
  data_ = other.data_;

  other.size_ = 0;
  other.n_data_ = 0;
  other.data_ = nullptr;
}

void BitMap::Resize(const usize size) {
  if (size == 0) {
    delete[] data_;
    size_ = 0;
    n_data_ = 0;
    data_ = nullptr;
    return;
  }

  const auto n_data = size % kBitMapDataBits == 0
                          ? (size / kBitMapDataBits)
                          : (size / kBitMapDataBits) + 1;
  const auto data = new u64[n_data];
  memset(data, 0, kBitMapDataBytes);

  if (data_) {
    memcpy(data, data_, kBitMapDataBytes * std::min(n_data, n_data_));
    delete[] data_;
  }

  size_ = size;
  n_data_ = n_data;
  data_ = data;
}

void BitMap::Set(const usize index, const bool value) const {
  const usize data_index = index / kBitMapDataBits;
  if (data_index >= size_)
    throw RuntimeException("index exceeds size of bitmap");

  if (value)
    data_[data_index] |= 1 << index % kBitMapDataBits;
  else
    data_[data_index] &= ~(1 << index % kBitMapDataBits);
}

bool BitMap::Get(const usize index) const {
  const usize data_index = index / kBitMapDataBits;
  if (data_index >= size_)
    throw RuntimeException("index exceeds size of bitmap");

  return (data_[data_index] & 1 << index % kBitMapDataBits) != 0;
}

} // namespace d0