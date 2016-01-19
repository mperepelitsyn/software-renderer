#include <algorithm>

#include "renderer/framebuffer.h"

namespace renderer {

void FrameBuffer::clear() {
  Vec4 black{0.0f, 0.0f, 0.0f, 1.0f};
  std::fill(color_.begin(), color_.end(), black);
  std::fill(depth_.begin(), depth_.end(), 1.f);
}

} // namespace renderer
