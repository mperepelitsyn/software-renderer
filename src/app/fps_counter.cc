#include <cassert>
#include <cmath>
#include <sstream>

#include "app/fps_counter.h"

namespace app {

void FPSCounter::tick(double delta) {
  assert(window_);

  ++frames_;
  elapsed_ += delta;

  if (elapsed_ >= freq_) {
    std::ostringstream oss{name_, std::ios_base::ate};
    auto fps = 1.0 / elapsed_ * frames_;

    oss << " [" << std::round(fps) << " fps, "
        << std::round(1000.0 / fps) << " ms]";
    glfwSetWindowTitle(window_, oss.str().c_str());

    frames_ = 0;
    elapsed_ = 0.0;
  }
}

} // namespace app
