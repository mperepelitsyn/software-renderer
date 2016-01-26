#include <algorithm>
#include <cstring>

#include "renderer/framebuffer.h"

namespace renderer {

void FrameBuffer::setPixel(unsigned x, unsigned y,
                           const Vec3 *color, unsigned count) {
  for (auto i = 0u; i < count; ++i)
    colors_[i]->setTexel(x, y, color[i]);
}

void FrameBuffer::clear() {
  Vec4 black{0.0f, 0.0f, 0.0f, 1.0f};
  for (auto &color : colors_)
    std::memset(color->getRawBuffer(), 0x0, color->getSize());
  std::fill(depth_.begin(), depth_.end(), 1.f);
}

} // namespace renderer
