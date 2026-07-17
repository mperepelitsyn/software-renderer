#pragma once

#include <string>

#include <SDL3/SDL.h>

namespace app {

class FPSCounter {
public:
  FPSCounter(const std::string &name, double output_freq) : name_{name}, freq_{output_freq} {}
  void tick(double delta);
  void setWindow(SDL_Window *window) { window_ = window; }

private:
  const std::string &name_;
  double freq_;
  SDL_Window *window_{};
  double elapsed_{};
  unsigned frames_{};
};

} // namespace app
