#include <algorithm>
#include <cstdlib>
#include <functional>

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

auto computeInvSlope(const Vertex &top, const Vertex &side) {
  return (side.position.x - top.position.x) / (top.position.y - side.position.y);
}

float getNearestPixelCenter(float x) {
  return floor(x) + 0.5;
}

float lerp(float a, float b, float w) {
  return (1 - w) * a + w * b;
}

Vec2 lerp(const Vec2 &v1, const Vec2 &v2, float t) {
  return {
    lerp(v1.x, v2.x, t),
    lerp(v1.y, v2.y, t),
  };
}

Vec3 lerp(const Vec3 &v1, const Vec3 &v2, float t) {
  return {
    lerp(v1.x, v2.x, t),
    lerp(v1.y, v2.y, t),
    lerp(v1.z, v2.z, t),
  };
}

Vec4 lerp(const Vec4 &v1, const Vec4 &v2, float t) {
  return {
    lerp(v1.x, v2.x, t),
    lerp(v1.y, v2.y, t),
    lerp(v1.z, v2.z, t),
    lerp(v1.w, v2.w, t),
  };
}

Vertex lerp(const Vertex &v1, const Vertex &v2, float t) {
  return {
    lerp(v1.position, v2.position, t),
    lerp(v1.color, v2.color, t),
    lerp(v1.normal, v2.normal, t),
    lerp(v1.tex_coord, v2.tex_coord, t),
  };
}

float getLerpT(float a, float b, float c) {
  return (c - a) / (b - a);
}

Vec2 getSlopes(const Vertex &v0, const Vertex &v1, const Vertex &v2) {
  Vec2 slope{computeInvSlope(v0, v1), computeInvSlope(v0, v2)};

  if (v1.position.x > v2.position.x)
    std::swap(slope.x, slope.y);

  return slope;
}

bool getEndpointsX(const Vec2 &slope, Vec2 &x, Vec2 &end,
                   float &lerp_x, float &lerp_step_x) {
    auto pix_left = getNearestPixelCenter(x.x);
    auto pix_right = getNearestPixelCenter(x.y);

    end.x = x.x <= pix_left ? pix_left : pix_left + 1;
    end.y = x.y > pix_right ? pix_right : pix_right - 1;

    lerp_x = getLerpT(x.x, x.y, end.x);
    lerp_step_x = getLerpT(x.x, x.y, end.x + 1) - lerp_x;

    x.x += slope.x;
    x.y += slope.y;

    return end.x <= end.y;
}

bool getEndpointsY(const Vertex &vert, const Vertex &base,
                   Vec2 &end, float &lerp_y, float &lerp_step_y,
                   bool bottom) {
  auto pix_vert = getNearestPixelCenter(vert.position.y);
  auto pix_base = getNearestPixelCenter(base.position.y);

  if (bottom) {
    end.x = vert.position.y > pix_vert ? pix_vert : pix_vert - 1;
    end.y = base.position.y <= pix_base ? pix_base : pix_base + 1;
  }
  else {
    end.x = base.position.y > pix_base ? pix_base : pix_base - 1;
    end.y = vert.position.y < pix_vert ? pix_vert : pix_vert + 1;
  }
  lerp_y = getLerpT(vert.position.y, base.position.y, end.x);
  lerp_step_y = getLerpT(vert.position.y, base.position.y, end.x - 1) - lerp_y;
  return end.x >= end.y;
}

Vec2 getInitialCoordX(const Vertex &v, const Vertex &left, const Vertex &right,
                      const Vec2 &slope, const Vec2 &y_end, bool bottom) {
  if (bottom)
    return {
      v.position.x + (v.position.y - y_end.x) * slope.x,
      v.position.x + (v.position.y - y_end.x) * slope.y
    };
  else
    return {
      left.position.x + (left.position.y - y_end.x) * slope.x,
      right.position.x + (left.position.y - y_end.x) * slope.y
    };
}

void rasterizeTopOrBottomTriangle(const Vertex &v, const Vertex &b1,
                                  const Vertex &b2,
                                  std::vector<Fragment> &fragments) {
  auto bottom = v.position.y > b1.position.y;
  auto slope = getSlopes(v, b1, b2);
  Vec2 end_y, end_x, lerp_t, lerp_step;
  if (!getEndpointsY(v, b1, end_y, lerp_t.y, lerp_step.y, bottom))
      return;

  auto is_v1_left = b1.position.x < b2.position.x;
  auto &left_base = is_v1_left ? b1 : b2;
  auto &right_base = is_v1_left ? b2 : b1;
  auto x = getInitialCoordX(v, left_base, right_base, slope, end_y, bottom);

  for (auto y = end_y.x; y >= end_y.y; --y) {
    if (!getEndpointsX(slope, x, end_x, lerp_t.x, lerp_step.x))
      continue;

    auto left = lerp(v, left_base, lerp_t.y);
    auto right = lerp(v, right_base, lerp_t.y);
    for (auto x = end_x.x; x <= end_x.y; ++x) {
      fragments.emplace_back(
          Vec3{x, y, 1.f}, lerp(left.color, right.color, lerp_t.x),
          lerp(left.normal, right.normal, lerp_t.x),
          lerp(left.tex_coord, right.tex_coord, lerp_t.x)
          );
      //fragments.emplace_back(lerp(left, right, lerp_t.x));
      lerp_t.x += lerp_step.x;
    }
    lerp_t.y += lerp_step.y;
  }
}

void rasterizeTriangle(std::vector<Fragment> &fragments, const Triangle &tri) {
  std::vector<Vertex> sorted{std::cbegin(tri.v), std::cend(tri.v)};
  std::sort(sorted.begin(), sorted.end(),
      [](Vertex &v1, Vertex &v2) { return v1.position.y > v2.position.y; });

  if (sorted[0].position.y == sorted[1].position.y)
    rasterizeTopOrBottomTriangle(sorted[2], sorted[0], sorted[1], fragments);
  else if (sorted[1].position.y == sorted[2].position.y)
    rasterizeTopOrBottomTriangle(sorted[0], sorted[1], sorted[2], fragments);
  else {
    auto t = getLerpT(sorted[0].position.y, sorted[2].position.y,
                      sorted[1].position.y);
    auto mid = lerp(sorted[0], sorted[2], t);
    rasterizeTopOrBottomTriangle(sorted[0], mid, sorted[1], fragments);
    rasterizeTopOrBottomTriangle(sorted[2], mid, sorted[1], fragments);
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
    else {
      rasterizeTriangle(fragments, tri);
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

