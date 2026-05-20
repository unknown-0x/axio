#ifndef AXIO_STRING_INTERNAL_BUFFER_HPP_
#define AXIO_STRING_INTERNAL_BUFFER_HPP_

#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>

#include "../base/macros.hpp"
#include "../base/types.hpp"
#include "../utility/move.hpp"

namespace axio {
static constexpr SizeT kInlineBufferSize = 512;

template <typename T, SizeT SIZE = kInlineBufferSize>
class BasicBuffer {
 public:
  using ValueType = T;
  using SizeType = SizeT;
  using Pointer = ValueType*;
  using ConstPointer = const ValueType*;

  BasicBuffer() noexcept : data_(stack_), size_(0), capacity_(SIZE) {}

  BasicBuffer(BasicBuffer&& other) noexcept { MoveFrom(axio::Move(other)); }
  BasicBuffer& operator=(BasicBuffer&& other) noexcept {
    if (this != &other) {
      Release();
      MoveFrom(axio::Move(other));
    }
    return *this;
  }

  BasicBuffer(const BasicBuffer&) = delete;
  BasicBuffer& operator=(const BasicBuffer&) = delete;

  ~BasicBuffer() { Release(); }

  Pointer Data() noexcept { return data_; }
  ConstPointer Data() const noexcept { return data_; }
  SizeType Size() const noexcept { return size_; }
  SizeType Capacity() const noexcept { return capacity_; }

  void Clear() noexcept { size_ = 0; }

  void Append(SizeType count, ValueType value) {
    if (count > capacity_ - size_) {
      Grow(ComputeCapacity(count));
    }
    std::memset(data_ + size_, value, count * sizeof(ValueType));
    size_ += count;
  }

  void Append(ConstPointer s, SizeType count) {
    if (count > capacity_ - size_) {
      Grow(ComputeCapacity(count));
    }
    std::memcpy(data_ + size_, s, count * sizeof(ValueType));
    size_ += count;
  }

  void Append(ConstPointer s) { Append(s, std::strlen(s)); }

  void Resize(SizeType new_size) {
    if (new_size > capacity_) {
      Grow(new_size);
    }
    size_ = new_size;
  }

  void Reserve(SizeType new_capacity) {
    if (new_capacity > capacity_) {
      Grow(new_capacity);
    }
  }

 private:
  SizeType ComputeCapacity(SizeType add_size) {
    static constexpr auto kMaxSz =
        std::numeric_limits<SizeType>::max() / sizeof(ValueType);

    const auto remaining = kMaxSz - capacity_;
    if (add_size > remaining) {
      throw std::bad_alloc();
    }
    const auto required = capacity_ + add_size;
    const auto new_capacity = capacity_ + capacity_ / 2;
    return AXIO_MAX(required, new_capacity);
  }

  void Grow(SizeType new_capacity) {
    auto new_data =
        static_cast<Pointer>(std::malloc(new_capacity * sizeof(ValueType)));
    AXIO_ASSERT(new_data != nullptr);
    std::memcpy(new_data, data_, size_ * sizeof(ValueType));
    Release();
    data_ = new_data;
    capacity_ = new_capacity;
  }

  void MoveFrom(BasicBuffer&& other) noexcept {
    if (other.data_ == other.stack_) {
      std::memcpy(stack_, other.stack_, other.size_ * sizeof(T));
      data_ = stack_;
    } else {
      data_ = other.data_;
      other.data_ = other.stack_;
    }

    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = other.stack_;
    other.size_ = 0;
    other.capacity_ = SIZE;
  }

  void Release() {
    if (data_ != stack_) {
      std::free(data_);
    }
  }

  T stack_[SIZE];

  Pointer data_;
  SizeType size_;
  SizeType capacity_;
};

template <SizeT SIZE = kInlineBufferSize>
using Buffer = BasicBuffer<char, SIZE>;
}  // namespace axio

#endif