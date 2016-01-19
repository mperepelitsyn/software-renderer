#pragma once

#include <vector>

#include "renderer/vector.h"

namespace renderer {

class FrameBuffer {
 public:
  FrameBuffer(unsigned width, unsigned height)
    : color_{width * height}, depth_(width * height),
      width_{width}, height_{height} {}
  void clear();
  Vec4 &getColor(unsigned x, unsigned y) { return color_[y * width_ + x]; }
  float &getDepth(unsigned x, unsigned y) { return depth_[y * width_ + x]; }
  const void *getColorBuffer() const { return &color_[0]; }
  const void *getDepthBuffer() const { return &depth_[0]; }
  unsigned getColorBufferSize() const { return color_.size() * sizeof(Vec4); }
  unsigned getDepthBufferSize() const { return color_.size() * sizeof(float); }
  unsigned getWidth() const { return width_; }
  unsigned getHeight() const { return height_; }

 private:
  std::vector<Vec4> color_;
  std::vector<float> depth_;
  unsigned width_;
  unsigned height_;
};

} // namespace renderer
