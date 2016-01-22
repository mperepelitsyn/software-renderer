#pragma once

#include <vector>
#include <memory>

#include "renderer/framebuffer.h"
#include "renderer/matrix.h"
#include "renderer/vector.h"

namespace renderer {

struct Attrs {
};

struct Vertex {
  Vec3 pos;
  std::unique_ptr<const Attrs> attrs;
};

struct VertexH {
  Vec4 pos;
  std::unique_ptr<const Attrs> attrs;
};

struct Fragment {
  Vec3 coord;
  std::unique_ptr<const Attrs> attrs;
};

using VertexShader = void(*)(const Vertex &in, const void *u, VertexH &out);
using FragmentShader = void(*)(const Fragment &in, const void *u, Vec4 &out);

struct Triangle {
  VertexH *v[3];
  float darea;
};

std::vector<VertexH> invokeVertexShader(const std::vector<Vertex> *vertices,
                                        const void *uniform,
                                        VertexShader shader);
std::vector<Triangle> assembleTriangles(std::vector<VertexH> &vertices);
std::vector<Triangle> clipTriangles(const std::vector<Triangle> &triangles);
std::vector<Triangle> cullBackFacing(const std::vector<Triangle> &triangles);
void convertToScreenSpace(std::vector<Triangle> &triangles,
                          unsigned width, unsigned height);
std::vector<Fragment> rasterize(const std::vector<Triangle> &triangles,
                                unsigned attr_size,
                                bool wireframe);
void invokeFragmentShader(const std::vector<Fragment> &frags,
                          FrameBuffer &fb,
                          const void *uniform,
                          FragmentShader shader);

} // namespace renderer
