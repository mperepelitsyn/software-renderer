#pragma once

#include <cassert>

namespace renderer {

class Arena {
 public:
  Arena() = default;
  Arena(unsigned count, unsigned size)
    : storage_{std::make_unique<char[]>(count * size)},
      ptr_{storage_.get()}, size_{size} {}

  void reset(unsigned count, unsigned size) {
    if (count * size > count_ * size_) {
      storage_ = std::make_unique<char[]>(count * size);
      count_ = count;
      size_ = size;
    }
    ptr_ = storage_.get();
  }

  template<class T>
  T *allocate() {
    assert(ptr_);
    auto p = reinterpret_cast<T*>(ptr_);
    ptr_ += size_;
    return p;
  }

 private:
  std::unique_ptr<char[]> storage_;
  char *ptr_{nullptr};
  unsigned size_{};
  unsigned count_{};
};

} // namespace renderer
