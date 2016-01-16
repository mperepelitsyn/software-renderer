#pragma once

#include <vector>

#include "renderer/framebuffer.h"
#include "renderer/vector.h"
#include "renderer/matrix.h"

namespace renderer {

struct Vertex {
  Vertex() {}
  Vertex(const Vec4 &pos) : position{pos} {}
  Vertex(const Vec4 &pos, const Vec3 color) : position{pos}, color{color} {}
  Vertex(const Vec4 &pos, const Vec3 &norm, const Vec2 &tc)
    : position{pos}, normal{norm}, tex_coord{tc} {}
  Vertex(const Vec4 &pos, const Vec3 &color, const Vec3 &norm, const Vec2 &tc)
    : position{pos}, color{color}, normal{norm}, tex_coord{tc} {}

  Vec4 position;
  Vec3 color;
  Vec3 normal;
  Vec2 tex_coord;
};

struct Fragment {
  Fragment(const Vec3 &coord) : frag_coord{coord} {}
  Fragment(const Vertex &v)
    : frag_coord{v.position.x, v.position.y, v.position.z},
      color{v.color}, normal{v.normal}, tex_coord{v.tex_coord} {}
  Fragment(const Vec3 &pos, const Vec3 &color, const Vec3 &norm, const Vec2 &tc)
    : frag_coord{pos}, color{color}, normal{norm}, tex_coord{tc} {}

  Vec3 frag_coord;
  Vec3 color;
  Vec3 normal;
  Vec2 tex_coord;
};

struct Uniform {
  Mat4 mvp;
};

using VertexShader = void(*)(const Vertex &in, const Uniform &u, Vertex &out);
using FragmentShader = void(*)(const Fragment &in, const Uniform &u, Vec4 &out);

struct Triangle {
  Vertex v[3];
};

std::vector<Vertex> invokeVertexShader(const std::vector<Vertex> &vertices,
                                       const Uniform &uniform,
                                       VertexShader shader);
std::vector<Triangle> assembleTriangles(const std::vector<Vertex> &vertices);
std::vector<Triangle> clipTriangles(const std::vector<Triangle> &triangles);
std::vector<Triangle> cullBackFacing(const std::vector<Triangle> &triangles);
void convertToScreenSpace(std::vector<Triangle> &triangles,
                          unsigned width, unsigned height);
std::vector<Fragment> rasterize(const std::vector<Triangle> &triangles,
                                bool wireframe);
void invokeFragmentShader(const std::vector<Fragment> &frags,
                          FrameBuffer &fb,
                          const Uniform &uniform,
                          FragmentShader shader);

} // namespace renderer