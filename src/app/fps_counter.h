#pragma once

#include <string>

#include <GLFW/glfw3.h>

namespace app {

class FPSCounter {
 public:
  FPSCounter(const std::string &name, double output_freq)
    : name_{name}, freq_{output_freq} {}
  void tick(double delta);
  void setWindow(GLFWwindow *window) { window_ = window; }

 private:
  const std::string &name_;
  double freq_;
  GLFWwindow *window_{};
  double elapsed_{};
  unsigned frames_{};
};

} // namespace app
