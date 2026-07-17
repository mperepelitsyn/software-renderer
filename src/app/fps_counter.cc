#include <cassert>
#include <format>

#include "app/fps_counter.h"

namespace app {

void FPSCounter::tick(double delta) {
  assert(window_);

  ++frames_;
  elapsed_ += delta;

  if (elapsed_ >= freq_) {
    auto fps = 1.0 / elapsed_ * frames_;
    auto title = std::format("{} [{:.0f} fps, {:.0f} ms]", name_, fps, 1000.0 / fps);
    SDL_SetWindowTitle(window_, title.c_str());

    frames_ = 0;
    elapsed_ = 0.0;
  }
}

} // namespace app
