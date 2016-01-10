#pragma once

#include <vector>

#include "vector.h"

class FrameBuffer {
 public:
  FrameBuffer(unsigned width, unsigned height)
    : buffer_{width * height}, width_{width}, height_{height} {}
  void clear();
  Vec4 &getColor(unsigned x, unsigned y) { return buffer_[y * width_ + x]; }
  const void *getRawBuffer() const { return &buffer_[0]; }
  const auto &getBuffer() const { return buffer_; }
  unsigned getBufferSize() const { return buffer_.size() * sizeof(Vec4); }
  unsigned getWidth() const { return width_; }
  unsigned getHeight() const { return height_; }

 private:
  std::vector<Vec4> buffer_;
  unsigned width_;
  unsigned height_;
};
