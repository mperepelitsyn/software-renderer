#pragma once

#include <vector>
#include <memory>

#include "renderer/arena.h"
#include "renderer/framebuffer.h"
#include "renderer/matrix.h"
#include "renderer/vector.h"

namespace renderer {

struct Vertex {
  Vec3 pos;
};

struct VertexH {
  Vec4 pos;
};

struct Fragment {
  // TODO: Needed only in line rasterization.
  Fragment(const Vec3 &coord) : coord{coord} {}
  Vec3 coord;
};

struct VertexBuffer {
  const void *ptr;
  size_t count;
  unsigned stride;
};

using VertexShader = void(*)(const Vertex &in, const void *u, VertexH &out);
using FragmentShader = void(*)(const Fragment &in, const void *u, Vec3 *out);

struct Program {
  VertexShader vs;
  FragmentShader fs;
  unsigned attr_count;
  unsigned color_count;
};

struct Triangle {
  VertexH *v[3];
  float darea;
};

class Pipeline {
 public:
  enum Culling { NONE, FRONT_FACING, BACK_FACING };

  void setVertexBuffer(const VertexBuffer *vb) { vb_ = vb; }
  void setFrameBuffer(FrameBuffer *fb) { fb_ = fb; }
  auto getFrameBuffer() { return fb_; }
  void setWireframeMode(bool mode) { wireframe_ = mode; }
  void setUniform(const void *u) { uniform_ = u; }
  void setProgram(const Program *program) { prog_ = program; }
  void setCulling(Culling mode) { culling_ = mode; }
  void draw();

  constexpr static unsigned max_fragment_size{32}; // In floats.
  constexpr static unsigned max_color_outputs{4};

 private:
  std::vector<VertexH*> invokeVertexShader();
  std::vector<Triangle> assembleTriangles(const std::vector<VertexH*> &vertices);
  std::vector<Triangle> clipTriangles(const std::vector<Triangle> &triangles);
  std::vector<Triangle> cullTriangles(const std::vector<Triangle> &triangles);
  void convertToScreenSpace(std::vector<Triangle> &triangles,
                            unsigned width, unsigned height);
  void rasterize(const std::vector<Triangle> &triangles);
  void rasterizeLine(const VertexH &v0, const VertexH &v1);
  void rasterizeTriHalfSpace(const Triangle &tri);
  void fill(const VertexH &v1, const VertexH &v2, float x, float y,float w);
  void fill(const Triangle &tri, float x, float y,
            float w0, float w1, float w2);
  void invokeFragmentShader(const Fragment &frag);

  Arena arena_;
  const VertexBuffer *vb_{nullptr};
  FrameBuffer *fb_{nullptr};
  const Program *prog_;
  const void *uniform_{nullptr};
  Culling culling_{NONE};
  bool wireframe_{false};
};



} // namespace renderer
