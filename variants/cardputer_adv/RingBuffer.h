#pragma once
#include <cstddef>

template <typename T, size_t Size> class RingBuffer {
private:
  T buf[Size];
  size_t head_idx;
  size_t tail_idx;
  bool is_full;

public:
  RingBuffer() : head_idx(0), tail_idx(0), is_full(false) {}

  // Nothing cleared, but index is reset
  inline void clear() {
    head_idx = 0;
    tail_idx = 0;
    is_full = false;
  }

  // Push element using copy
  void push(T data) {
    if (is_full) {
      tail_idx = (tail_idx + 1) % Size;
    }

    buf[head_idx] = data;
    head_idx = (head_idx + 1) % Size;

    if (head_idx == tail_idx) {
      is_full = true;
    }
  }

  // Not exactly push, but returns the address of the element to push and updates the index, we can use it to modify the element
  T* push_ref() {
    if (is_full) {
      tail_idx = (tail_idx + 1) % Size;
    }

    T* ptr = &buf[head_idx];

    head_idx = (head_idx + 1) % Size;

    if (head_idx == tail_idx) {
      is_full = true;
    }

    return ptr;
  }

  bool get(size_t index, const T* &data) const {
    if (index >= count()) {
      return false;
    }

    size_t real_idx = (tail_idx + index) % Size;
    data = &buf[real_idx];
    return true;
  }

  size_t count() const {
    if (is_full) {
      return Size;
    }

    if (head_idx >= tail_idx) {
      return head_idx - tail_idx;
    }

    return Size + head_idx - tail_idx;
  }
};
