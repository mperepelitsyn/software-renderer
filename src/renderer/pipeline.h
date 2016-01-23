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
  unsigned count;
  unsigned stride;
};

using VertexShader = void(*)(const Vertex &in, const void *u, VertexH &out);
using FragmentShader = void(*)(const Fragment &in, const void *u, Vec4 &out);

struct Program {
  VertexShader vs;
  FragmentShader fs;
  unsigned attr_count;
};

struct Triangle {
  VertexH *v[3];
  float darea;
};

class Pipeline {
 public:
  void setVertexBuffer(const VertexBuffer *vb) { vb_ = vb; }
  void setFrameBuffer(FrameBuffer &fb) { fb_ = &fb; }
  void setWireframeMode(bool mode) { wireframe_ = mode; }
  void setUniform(const void *u) { uniform_ = u; }
  void setProgram(const Program *program) { prog_ = program; }
  void draw();

 private:
  std::vector<VertexH*> invokeVertexShader();
  std::vector<Triangle> assembleTriangles(const std::vector<VertexH*> &vertices);
  std::vector<Triangle> clipTriangles(const std::vector<Triangle> &triangles);
  std::vector<Triangle> cullBackFacing(const std::vector<Triangle> &triangles);
  void convertToScreenSpace(std::vector<Triangle> &triangles,
                            unsigned width, unsigned height);
  std::vector<std::unique_ptr<const Fragment>> rasterize(
      const std::vector<Triangle> &triangles);
  void invokeFragmentShader(
      const std::vector<std::unique_ptr<const Fragment>> &frags);

  Arena arena_;
  const VertexBuffer *vb_{nullptr};
  FrameBuffer *fb_{nullptr};
  const Program *prog_;
  const void *uniform_{nullptr};
  bool wireframe_{false};
};



} // namespace renderer
