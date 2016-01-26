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
#version 450 core

const vec2 pos[4] = {{-1.0, -1.0}, {1.0, -1.0}, {-1.0, 1.0}, {1.0, 1.0}};
void main(void) {
  gl_Position = vec4(pos[gl_VertexID], 0.5, 1.0);
}
)";

const GLchar *fs_source = R"(
#version 450 core

layout (binding = 0) uniform sampler2D sampler;
out vec4 color;

void main(void) {
  color = texelFetch(sampler, ivec2(gl_FragCoord.xy), 0);
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
  : fb_{w, h, 1}, color_buf_{w, h}, width_{w}, height_{h}, name_{name},
    fps_counter_{name_, 0.25} {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit())
    throw Error{"failed to initialize glfw"};

  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
  window_ = glfwCreateWindow(w, h, name_.c_str(), nullptr, nullptr);
  if (!window_) {
    glfwTerminate();
    throw Error{"failed to create glfw window"};
  }

  glfwMakeContextCurrent(window_);
  glfwSetKeyCallback(window_, keyCallback);

  glewInit();

  glCreateTextures(GL_TEXTURE_2D, 1, &texture_);
  glTextureStorage2D(texture_, 1, GL_RGB32F, width_, height_);
  glBindTextureUnit(0, texture_);
  program_ = linkProgram(vs_source, fs_source);
  glUseProgram(program_);

  fb_.attachColor(0, &color_buf_);
  ctx_.setFrameBuffer(&fb_);
  fps_counter_.setWindow(window_);
}

App::~App() {
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
    glTextureSubImage2D(texture_, 0, 0, 0, width_, height_, GL_RGB, GL_FLOAT,
                        color_buf_.getRawBuffer());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glfwSwapBuffers(window_);
    glfwPollEvents();
  }
  shutdown();
}

} // namespace app

