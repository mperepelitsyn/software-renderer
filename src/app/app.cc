#include <sstream>

#include "app/app.h"

namespace app {

namespace {

void errorCallback(int error, const char *desc) {
  std::ostringstream oss;
  oss << "GLFW error: (" << error << ") " << desc;
  throw Error{oss.str()};
}

void keyCallback(GLFWwindow* window, int key, int /*sc*/, int action,
                 int /*mods*/) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

const GLchar *vs_source = R"(
#version 410 core

const vec2 pos[4] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0),
                           vec2(-1.0, 1.0), vec2(1.0, 1.0));
out vec2 uv;

void main(void) {
  uv = pos[gl_VertexID] * 0.5 + 0.5;
  gl_Position = vec4(pos[gl_VertexID], 0.5, 1.0);
}
)";

const GLchar *fs_source = R"(
#version 410 core

uniform sampler2D tex;
in vec2 uv;
out vec4 color;

void main(void) {
  color = texture(tex, uv);
}
)";

GLuint compileShader(GLuint type, const GLchar *source) {
  auto shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  return shader;
}

GLuint linkProgram(const GLchar *vs_source, const GLchar *fs_source) {
  auto program = glCreateProgram();
  auto vs = compileShader(GL_VERTEX_SHADER, vs_source);
  auto fs = compileShader(GL_FRAGMENT_SHADER, fs_source);
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

} // namespace

App::App(unsigned w, unsigned h, const std::string &name)
  : fb_{w, h}, width_{w}, height_{h}, name_{name}, fps_counter_{name_, 0.25} {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit())
    throw Error{"failed to initialize glfw"};

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  window_ = glfwCreateWindow(w, h, name_.c_str(), nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    throw Error{"failed to create glfw window"};
  }

  glfwMakeContextCurrent(window_);
  glfwSetKeyCallback(window_, keyCallback);

  if (!gladLoadGL(glfwGetProcAddress))
    throw Error{"failed to load OpenGL"};

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);

  glGenTextures(1, &texture_);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  program_ = linkProgram(vs_source, fs_source);
  glUseProgram(program_);

  ctx_.setFrameBuffer(&fb_);
  fps_counter_.setWindow(window_);
}

App::~App() {
  glDeleteVertexArrays(1, &vao_);
  glDeleteTextures(1, &texture_);
  glDeleteProgram(program_);
  glfwDestroyWindow(window_);
  glfwTerminate();
}

void App::render() {
  startup();
  while(!glfwWindowShouldClose(window_)) {
    auto time = glfwGetTime();
    auto delta = time - last_time_;
    last_time_ = time;

    fps_counter_.tick(delta);
    renderLoop(time, delta);

    glClear(GL_COLOR_BUFFER_BIT);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA,
                    GL_UNSIGNED_BYTE, fb_.getColorTexture().getRawBuffer());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
  shutdown();
}

} // namespace app

