#include <cstdlib>

#include "pipeline.h"

namespace {

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

// Bresenham's line algorithm.
void rasterizeLine(std::vector<Fragment> &frags, const Vertex &v0,
                   const Vertex &v1) {
  int x0 = v0.position.x;
  int x1 = v1.position.x;
  int y0 = v0.position.y;
  int y1 = v1.position.y;
  auto steep = false;
  if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
    steep = true;
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  int y = y0;
  int dx = x1 - x0;
  int dy = std::abs(y1 - y0);
  auto diff = 2 * dy - dx;
  int y_growth = y1 > y0 ? 1 : -1;

  if (steep)
    frags.emplace_back(Vec3(y0, x0, v0.position.z));
  else
    frags.emplace_back(Vec3(x0, y0, v0.position.z));

  if (diff > 0) {
    y += y_growth;
    diff -= 2 * dx;
  }
  for (int x = x0 + 1; x <= x1; ++x) {
    // TODO: Interpolate later.
    if (steep)
      frags.emplace_back(Vec3(y, x, 1.0f));
    else
      frags.emplace_back(Vec3(x, y, 1.0f));

    diff += 2 * dy;
    if (diff > 0) {
      y += y_growth;
      diff -= 2 * dx;
    }
  }
}

} // namespace

std::vector<Vertex> invokeVertexShader(const std::vector<Vertex> &vertices,
                                       VertexShader shader) {
  std::vector<Vertex> transformed{vertices.size()};
  for (auto i = 0u; i < vertices.size(); ++i) {
    shader(vertices[i], transformed[i]);
  }
  return transformed;
}

std::vector<Triangle> assembleTriangles(const std::vector<Vertex> &vertices) {
  std::vector<Triangle> triangles{vertices.size() / 3};
  for (auto i = 0u; i < vertices.size(); i += 3) {
    triangles[i].v[0] = vertices[i];
    triangles[i].v[1] = vertices[i + 1];
    triangles[i].v[2] = vertices[i + 2];
  }
  return triangles;
}

std::vector<Triangle> clipTriangles(const std::vector<Triangle> &triangles) {
  return triangles;
}

std::vector<Triangle> cullBackFacing(const std::vector<Triangle> &triangles) {
  return triangles;
}

void convertToScreenSpace(std::vector<Triangle> &triangles,
                          unsigned width, unsigned height) {
  for (auto &tri : triangles) {
    for (auto &vert : tri.v) {
      vert.position.x = vert.position.x * (width - 1) / 2 + (width - 1) / 2;
      vert.position.y = vert.position.y * (height - 1) / 2 + (height - 1) / 2;
    }
  }
}

std::vector<Fragment> rasterize(const std::vector<Triangle> &triangles,
                                bool wireframe) {
  std::vector<Fragment> fragments;
  for (auto &tri : triangles) {
    if (wireframe) {
      rasterizeLine(fragments, tri.v[0], tri.v[1]);
      rasterizeLine(fragments, tri.v[0], tri.v[2]);
      rasterizeLine(fragments, tri.v[1], tri.v[2]);
    }
  }
  return fragments;
}

void invokeFragmentShader(const std::vector<Fragment> &fragments,
                          FrameBuffer &fb,
                          FragmentShader shader) {
  for (auto &frag : fragments) {
    shader(frag, fb.getColor(frag.frag_coord.x, frag.frag_coord.y));
  }
}

