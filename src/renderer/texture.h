#pragma once

#include <string>
#include <vector>

#include "renderer/vector.h"

namespace renderer {

class Texture {
 public:
  Texture(short width, short height, const std::vector<Vec3> &buf)
    : buffer_{buf}, width_{width}, height_{height} {}

  Vec3 sample(float u, float v) const
    { return fetchTexel(u * (width_ - 1), v * (height_ - 1)); }

  Vec3 fetchTexel(unsigned x, unsigned y) const
    { return buffer_[y * width_ + x]; }

 private:
  std::vector<Vec3> buffer_;
  short width_;
  short height_;
};

} // namespace renderer
