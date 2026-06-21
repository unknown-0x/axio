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
/** Number of elements held inline (on the stack) before BasicBuffer
    spills onto the heap. */
static constexpr SizeT kInlineBufferSize = 512;

/**
 * @brief A move-only, append-oriented buffer with small-buffer
 *        optimization.
 *
 * BasicBuffer stores up to @p SIZE elements inline in `stack_`. Once
 * that capacity is exceeded it transparently switches to a heap
 * allocation obtained via `std::malloc`, growing geometrically as
 * needed. It is not copyable, only movable, and is intended as a
 * short-lived scratch buffer rather than a general-purpose container.
 *
 * @tparam T    Element type. Must be trivially copyable for the
 *              `std::memcpy`/`std::memset`-based operations to be valid.
 * @tparam SIZE Number of elements stored inline before heap allocation
 *              occurs. Defaults to kInlineBufferSize.
 */
template <typename T, SizeT SIZE = kInlineBufferSize>
class BasicBuffer {
 public:
  using ValueType = T;
  using SizeType = SizeT;
  using Pointer = ValueType*;
  using ConstPointer = const ValueType*;

  /// Constructs an empty buffer using the inline (stack) storage.
  BasicBuffer() noexcept : data_(stack_), size_(0), capacity_(SIZE) {}

  /**
   * @brief Move-constructs from @p other, stealing its heap allocation
   *        if it has one, or copying its inline bytes otherwise.
   * @param other Buffer to move from; left empty and pointing at its own
   *              inline storage afterward.
   */
  BasicBuffer(BasicBuffer&& other) noexcept { MoveFrom(axio::Move(other)); }

  /**
   * @brief Move-assigns from @p other, releasing any existing heap
   *        allocation first.
   * @param other Buffer to move from; left empty afterward.
   * @return Reference to `*this`.
   */
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

  /// @return Mutable pointer to the start of the buffer's contents.
  Pointer Data() noexcept { return data_; }
  /// @return Read-only pointer to the start of the buffer's contents.
  ConstPointer Data() const noexcept { return data_; }
  /// @return Number of elements currently stored.
  SizeType Size() const noexcept { return size_; }
  /// @return Number of elements that can be stored without reallocating.
  SizeType Capacity() const noexcept { return capacity_; }

  /// Resets the logical size to zero without releasing any allocation.
  void Clear() noexcept { size_ = 0; }

  /**
   * @brief Appends @p count copies of @p value, growing the buffer first
   *        if necessary.
   * @param count Number of copies to append.
   * @param value Byte value used to fill the appended region (passed to
   *              `std::memset`, so only its low-order byte applies).
   */
  void Append(SizeType count, ValueType value) {
    if (count > capacity_ - size_) {
      Grow(ComputeCapacity(count));
    }
    std::memset(data_ + size_, value, count * sizeof(ValueType));
    size_ += count;
  }

  /**
   * @brief Appends @p count elements copied from @p s, growing the
   *        buffer first if necessary.
   * @param s     Pointer to the elements to copy in.
   * @param count Number of elements to copy.
   */
  void Append(ConstPointer s, SizeType count) {
    if (count > capacity_ - size_) {
      Grow(ComputeCapacity(count));
    }
    std::memcpy(data_ + size_, s, count * sizeof(ValueType));
    size_ += count;
  }

  /**
   * @brief Appends a null-terminated string, using `std::strlen` to
   *        determine its length.
   * @param s Null-terminated string to append.
   */
  void Append(ConstPointer s) { Append(s, std::strlen(s)); }

  /**
   * @brief Sets the logical size to @p new_size, growing the underlying
   *        storage first if @p new_size exceeds the current capacity.
   *
   * Does not initialize any newly exposed elements.
   *
   * @param new_size New logical size, in elements.
   */
  void Resize(SizeType new_size) {
    if (new_size > capacity_) {
      Grow(new_size);
    }
    size_ = new_size;
  }

  /**
   * @brief Ensures capacity for at least @p new_capacity elements,
   *        growing the underlying storage if needed. Leaves Size()
   *        unchanged.
   * @param new_capacity Minimum capacity to guarantee, in elements.
   */
  void Reserve(SizeType new_capacity) {
    if (new_capacity > capacity_) {
      Grow(new_capacity);
    }
  }

 private:
  /**
   * @brief Computes the next capacity to grow to when at least
   *        @p add_size additional elements are needed.
   *
   * Grows geometrically (1.5x) but never below the amount strictly
   * required to fit the requested addition.
   *
   * @param add_size Number of additional elements that must fit.
   * @return The new capacity, in elements.
   * @throws std::bad_alloc if the required capacity would overflow
   *         SizeType arithmetic.
   */
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

  /**
   * @brief Allocates a new heap buffer of @p new_capacity elements,
   *        copies the existing contents into it, and releases the
   *        previous allocation (if any).
   * @param new_capacity New capacity, in elements; must be at least
   *                     Size().
   */
  void Grow(SizeType new_capacity) {
    auto new_data =
        static_cast<Pointer>(std::malloc(new_capacity * sizeof(ValueType)));
    AXIO_ASSERT(new_data != nullptr);
    std::memcpy(new_data, data_, size_ * sizeof(ValueType));
    Release();
    data_ = new_data;
    capacity_ = new_capacity;
  }

  /**
   * @brief Takes ownership of @p other's storage (heap pointer or
   *        inline bytes) and resets @p other to an empty, inline state.
   * @param other Source buffer being moved from.
   */
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

/**
 * @brief Convenience alias for a `char`-based BasicBuffer, as used by the
 *        `AxioRepr` string-formatting utilities.
 * @tparam SIZE Inline capacity, in characters. Defaults to
 *              kInlineBufferSize.
 */
template <SizeT SIZE = kInlineBufferSize>
using Buffer = BasicBuffer<char, SIZE>;
}  // namespace axio

#endif