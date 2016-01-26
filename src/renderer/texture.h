#pragma once

#include <vector>

#include "renderer/vector.h"

namespace renderer {

class Texture {
 public:
  Texture(unsigned width, unsigned height, const std::vector<Vec3> &buf)
    : buffer_{buf}, width_{width}, height_{height} {}

  Texture(unsigned width, unsigned height)
    : buffer_{width * height}, width_{width}, height_{height} {}

  Vec3 sample(float u, float v) const
    { return fetchTexel(u * (width_ - 1), v * (height_ - 1)); }

  Vec3 fetchTexel(unsigned x, unsigned y) const
    { return buffer_[y * width_ + x]; }

  void setTexel(unsigned x, unsigned y, const Vec3 &color)
    { buffer_[y * width_ + x] = color; }

  auto getSize() const { return width_ * height_ * sizeof buffer_[0]; }
  void *getRawBuffer() { return &buffer_[0]; }

 private:
  std::vector<Vec3> buffer_;
  unsigned width_;
  unsigned height_;
};

} // namespace renderer
