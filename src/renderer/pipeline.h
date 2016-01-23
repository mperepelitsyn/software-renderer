#pragma once

#include <vector>
#include <memory>

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

std::vector<std::unique_ptr<VertexH>> invokeVertexShader(const VertexBuffer &vb,
    const Program &prog, const void *uniform);

std::vector<Triangle> assembleTriangles(
    std::vector<std::unique_ptr<VertexH>> &vertices);

std::vector<Triangle> clipTriangles(const std::vector<Triangle> &triangles);

std::vector<Triangle> cullBackFacing(const std::vector<Triangle> &triangles);

void convertToScreenSpace(std::vector<Triangle> &triangles,
                          unsigned width, unsigned height);

std::vector<std::unique_ptr<const Fragment>> rasterize(
    const std::vector<Triangle> &triangles, unsigned attr_count,
    bool wireframe);

void invokeFragmentShader(
    const std::vector<std::unique_ptr<const Fragment>> &frags,
    FrameBuffer &fb, const void *uniform, FragmentShader shader);

} // namespace renderer
