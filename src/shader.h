#pragma once

#include <vector>

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
  Vec3 frag_coord;
  Vec3 normal;
  Vec2 tex_coord;
};

using Color = Vec4;

using VertexShader = void(*)(const Vertex &in, Vertex &out);
using FragmentShader = void(*)(const Fragment &in, Color &out);

struct VertexShaderInvocation {
  VertexShaderInvocation(const Vertex &in, Vertex &out) : in{&in}, out{&out} {}

  const Vertex *in;
  Vertex *out;
  VertexShader shader;
};

struct FragmentShaderInvocation {
  FragmentShaderInvocation(const Fragment &in, Color &out) : in{&in}, out{&out} {}

  const Fragment *in;
  Color *out;
  FragmentShader shader;
};

void vsPassThrough(const Vertex &, Vertex &) {}

void fsPassThrough(const Fragment &, Color &out) {
  out = Vec4(1.f, 1.f, 1.f, 1.f);
}

