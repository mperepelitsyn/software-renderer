#pragma once

#include <vector>

#include "renderer/texture.h"

namespace renderer {

class FrameBuffer {
 public:
  FrameBuffer(unsigned width, unsigned height, unsigned color_count)
    : colors_{color_count}, depth_(width * height),
      width_{width}, height_{height} {}

  void clear();
  void attachColor(unsigned slot, Texture *tex) { colors_.at(slot) = tex; }
  void setPixel(unsigned x, unsigned y, const Vec3 *color, unsigned count);
  float &getDepth(unsigned x, unsigned y) { return depth_[y * width_ + x]; }
  unsigned getWidth() const { return width_; }
  unsigned getHeight() const { return height_; }

 private:
  std::vector<Texture*> colors_;
  std::vector<float> depth_;
  unsigned width_;
  unsigned height_;
};

} // namespace renderer
