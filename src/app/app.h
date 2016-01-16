#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "app/error.h"
#include "renderer/context.h"

#define DEFINE_AND_CALL_APP(app_type, w, h, title) \
int main() { \
  try { \
    app_type _app(w, h, #title); \
    _app.render(); \
  } \
  catch (const app::Error &e) { \
    std::cerr << e.what() << '\n'; \
    return EXIT_FAILURE; \
  } \
}

namespace app {

class App {
 public:
  App(unsigned w, unsigned h, const std::string& name);
  void render();
  ~App();

 protected:
  virtual void renderLoop(double time, double delta) = 0;
  virtual void startup() {}
  virtual void shutdown() {}
  void setTitle(const std::string &title);

  renderer::Context ctx_;
  renderer::FrameBuffer fb_;
  unsigned width_, height_;

 private:
  GLFWwindow *window_;
  double last_time_;
  GLuint program_;
  GLuint texture_;
  std::string name_;
};

} // namespace app

