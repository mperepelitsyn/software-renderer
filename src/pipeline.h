#pragma once

#include <vector>

#include "framebuffer.h"
#include "vector.h"

struct Vertex {
  Vertex() {}
  Vertex(const Vec4 &pos) : position{pos} {}
  Vertex(const Vec4 &pos, const Vec3 color) : position{pos}, color{color} {}
  Vertex(const Vec4 &pos, const Vec3 &norm, const Vec2 &tc)
    : position{pos}, normal{norm}, tex_coord{tc} {}

  Vec4 position;
  Vec3 color;
  Vec3 normal;
  Vec2 tex_coord;
};

struct Fragment {
  Fragment(const Vec3 &coord) : frag_coord{coord} {}

  Vec3 frag_coord;
  Vec3 normal;
  Vec2 tex_coord;
};

using Color = Vec4;
using VertexShader = void(*)(const Vertex &in, Vertex &out);
using FragmentShader = void(*)(const Fragment &in, Color &out);

inline void vsPassThrough(const Vertex &in, Vertex &out) {
  out = in;
}

inline void fsPassThrough(const Fragment &, Color &out) {
  out = Vec4(1.f, 1.f, 1.f, 1.f);
}

struct Triangle {
  Vertex v[3];
};

std::vector<Vertex> invokeVertexShader(const std::vector<Vertex> &vertices,
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
                          FragmentShader shader);

