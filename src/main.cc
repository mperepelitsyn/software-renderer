#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <fstream>

#include "context.h"

constexpr unsigned width = 640;
constexpr unsigned height = 640;

namespace {

void errorCallback(int error, const char *desc) {
  std::cerr << "GLFW error: (" << error << ") " << desc;
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

void dumpAsTGA(const std::string &path, const FrameBuffer &fb) {
#pragma pack(push, 1)
  struct {
    char  idlength;
    char  colourmaptype;
    char  datatypecode;
    short int colourmaporigin;
    short int colourmaplength;
    char  colourmapdepth;
    short int x_origin;
    short int y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
  } tga_header {0, 0, 2, 0, 0, 0, 0, 0,
                (short)fb.getWidth(), (short)fb.getHeight(), 24, 0};
#pragma pack(pop)

  std::ofstream ofs(path, std::ios::binary);
  ofs.write(reinterpret_cast<char*>(&tga_header), sizeof(tga_header));
  for (auto &pix : fb.getBuffer()) {
    ofs.put(pix.b * 255);
    ofs.put(pix.g * 255);
    ofs.put(pix.r * 255);
  }
}

void *genTexture() {
  static Vec4 texture[width * height];
  for (auto &texel : texture)
    texel = Vec4(1.f, 1.f, 1.f, 1.f);
  return texture;
}

void vertex_shader(const Vertex &in, const Uniform *u, Vertex &out) {
  out.position = u->mvp * in.position;
  auto n = u->mvp * Vec4{in.normal, 0.f};
  out.normal = {n.x, n.y, n.z};
  out.color = in.color;
  out.tex_coord = in.tex_coord;
}

void fragment_shader(const Fragment &in, const Uniform *, Vec4 &out) {
  out = Vec4(in.color.r, in.color.g, in.color.b, 1.f);
}

} // namespace

int main(void)
{
  GLFWwindow* window;
  glfwSetErrorCallback(errorCallback);
  if (!glfwInit())
    exit(EXIT_FAILURE);
  window = glfwCreateWindow(width, height, "Software Renderer", NULL, NULL);
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glewInit();
  glfwSetKeyCallback(window, keyCallback);

  GLuint tex;
  glCreateTextures(GL_TEXTURE_2D, 1, &tex);
  glTextureStorage2D(tex, 1, GL_RGBA32F, width, height);
  glBindTextureUnit(0, tex);
  auto vs = glCreateShader(GL_VERTEX_SHADER);
  auto fs = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vs, 1, &vs_source, nullptr);
  glShaderSource(fs, 1, &fs_source, nullptr);
  glCompileShader(vs);
  glCompileShader(fs);
  auto program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glUseProgram(program);

  Context ctx;
  FrameBuffer fb{width, height};
  std::vector<Vertex> vertices{
    {{-.5f, .5f, .0f, 1.f}, {1.f, .0f, .0f}},
    {{.0f, -.5f, .0f, 1.f}, {0.f, 0.f, 1.f}},
    {{.5f, .5f, .0f, 1.f}, {0.f, 1.f, 0.f}},
  };
  ctx.setVertices(vertices);
  ctx.setFrameBuffer(fb);
  ctx.setVertexShader(vertex_shader);
  ctx.setFragmentShader(fragment_shader);

  auto view = createViewMatrix({0.f, 0.f, 2.f}, {0.f, 0.f, 0.f},
                               {0.f, 1.f, 0.f});
  auto proj = createPerspProjMatrix(70.0_deg,
      static_cast<float>(width) / height, 0.01f, 100.f);
  auto proj_view = proj * view;

  while (!glfwWindowShouldClose(window)) {
    float time = glfwGetTime();
    fb.clear();

    auto model = rotateZ(time) * translate({0.3f, 0.f, 0.f}) *
                 rotateZ(-time) * rotateY(time);
    Uniform u{proj_view * model};
    ctx.SetUniform(u);

    ctx.draw();

    glClear(GL_COLOR_BUFFER_BIT);
    glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT,
                        fb.getRawBuffer()); // genTexture());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteTextures(1, &tex);

  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
