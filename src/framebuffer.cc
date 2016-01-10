#include <algorithm>

#include "framebuffer.h"

void FrameBuffer::clear() {
  Vec4 black{0.0f, 0.0f, 0.0f, 1.0f};
  std::fill(buffer_.begin(), buffer_.end(), black);
}
