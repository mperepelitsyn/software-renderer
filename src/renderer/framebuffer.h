#pragma once

#include "renderer/texture.h"

namespace renderer {

class FrameBuffer {
 public:
  FrameBuffer(unsigned width, unsigned height)
    : color_{width, height}, depth_{width, height}, color_write_{true} {}

  void clear() {
    color_.clear();
    depth_.fill(1.f);
  }

  void setPixel(unsigned x, unsigned y, const Vec4 &color, float depth) {
    if (color_write_)
      color_.setTexel(x, y, color);
    depth_.setTexel(x, y, depth);
  }

  void setColorWrite(bool write) { color_write_ = write; }
  auto &getColorTexture() const { return color_; }
  auto &getDepthTexture() const { return depth_; }
  auto getDepth(unsigned x, unsigned y) { return depth_.fetchTexel(x, y); }
  auto getWidth() const { return color_.getWidth(); }
  auto getHeight() const { return color_.getHeight(); }

 private:
  Texture<UNorm> color_;
  Texture<float> depth_;
  bool color_write_;
};

} // namespace renderer
