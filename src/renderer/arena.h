#pragma once

#include <memory>

namespace renderer {

class Arena {
 public:
  Arena() = default;
  Arena(unsigned count, unsigned size, unsigned alignment)
    { reset(count, size, alignment); }

  void reset(unsigned count, unsigned size, unsigned alignment) {
    auto alloc_size = count * size + alignment - 1;
    if (size && alloc_size_ < alloc_size) {
      storage_ = std::make_unique<unsigned char[]>(alloc_size);
      alloc_size_ = alloc_size;
    }
    size_ = size;
    ptr_ = storage_.get();

    size_t space;
    std::align(alignment, alloc_size, ptr_, space);
  }

  template<class T>
  T *allocate() {
    auto p = ptr_;
    ptr_ = static_cast<unsigned char*>(ptr_) + size_;

    return static_cast<T*>(p);
  }

 private:
  std::unique_ptr<unsigned char[]> storage_;
  void *ptr_{nullptr};
  unsigned size_{};
  unsigned alloc_size_{};
};

} // namespace renderer
