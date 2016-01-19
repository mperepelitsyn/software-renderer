#include <algorithm>
#include <cstdlib>
#include <functional>

#include "renderer/pipeline.h"

namespace renderer {

namespace {

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

float getNearestPixelCenter(float x) {
  return floor(x) + 0.5;
}

Fragment interpolate(const Triangle &tri, float x, float y,
                     float w0, float w1, float w2) {
  auto z = w0 * tri.v[0].position.z + w1 * tri.v[1].position.z +
           w2 * tri.v[2].position.z;
  return {
    {x, y, z},
    (tri.v[0].color * w0 * tri.v[0].position.z +
     tri.v[1].color * w1 * tri.v[1].position.z +
     tri.v[2].color * w2 * tri.v[2].position.z) / z,
    (tri.v[0].normal * w0 * tri.v[0].position.z +
     tri.v[1].normal * w1 * tri.v[1].position.z +
     tri.v[2].normal * w2 * tri.v[2].position.z) / z,
    (tri.v[0].tex_coord * w0 * tri.v[0].position.z +
     tri.v[1].tex_coord * w1 * tri.v[1].position.z +
     tri.v[2].tex_coord * w2 * tri.v[2].position.z) / z,
    (tri.v[0].pos_view * w0 * tri.v[0].position.z +
     tri.v[1].pos_view * w1 * tri.v[1].position.z +
     tri.v[2].pos_view * w2 * tri.v[2].position.z) / z,
  };
}

// Top-left filling convention.
void rasterizeTriHalfSpace(const Triangle &tri, std::vector<Fragment> &frags) {
  auto x0 = tri.v[0].position.x;
  auto x1 = tri.v[1].position.x;
  auto x2 = tri.v[2].position.x;
  auto y0 = tri.v[0].position.y;
  auto y1 = tri.v[1].position.y;
  auto y2 = tri.v[2].position.y;
  auto aabb_x = std::minmax({x0, x1, x2});
  auto aabb_y = std::minmax({y0, y1, y2});
  auto e0_top_left = y1 > y2 || (y1 == y2 && x1 > x2);
  auto e1_top_left = y2 > y0 || (y2 == y0 && x2 > x0);
  auto e2_top_left = y0 > y1 || (y0 == y1 && x0 > x1);

  /*
  auto dx10 = x1 - x0;
  auto dx21 = x2 - x1;
  auto dx02 = x0 - x2;
  auto dy10 = y1 - y0;
  auto dy21 = y2 - y1;
  auto dy02 = y0 - y2;
  */

  for (auto y = getNearestPixelCenter(aabb_y.first);
       y <= getNearestPixelCenter(aabb_y.second); ++y) {
    for (auto x = getNearestPixelCenter(aabb_x.first);
         x <= getNearestPixelCenter(aabb_x.second); ++x) {
      auto e0 = (x2 - x1) * (y - y1) - (x - x1) * (y2 - y1);
      auto e1 = (x0 - x2) * (y - y2) - (x - x2) * (y0 - y2);
      auto e2 = (x1 - x0) * (y - y0) - (x - x0) * (y1 - y0);

      if ((e0 > 0 || (e0 == 0 && e0_top_left)) &&
          (e1 > 0 || (e1 == 0 && e1_top_left)) &&
          (e2 > 0 || (e2 == 0 && e2_top_left))) {
        auto w0 = e0 / tri.darea;
        auto w1 = e1 / tri.darea;
        auto w2 = 1 - w0 - w1;

        frags.emplace_back(interpolate(tri, x, y, w0, w1, w2));
      }
    }
  }
}

} // namespace

std::vector<Vertex> invokeVertexShader(const std::vector<Vertex> &vertices,
                                       const Uniform &uniform,
                                       VertexShader shader) {
  std::vector<Vertex> transformed{vertices.size()};
  for (auto i = 0u; i < vertices.size(); ++i) {
    shader(vertices[i], uniform, transformed[i]);
  }
  return transformed;
}

std::vector<Triangle> assembleTriangles(const std::vector<Vertex> &vertices) {
  auto tri_count = vertices.size() / 3;
  std::vector<Triangle> triangles{tri_count};
  for (auto i = 0u; i < tri_count; ++i) {
    triangles[i].v[0] = vertices[i * 3];
    triangles[i].v[1] = vertices[i * 3 + 1];
    triangles[i].v[2] = vertices[i * 3 + 2];
  }
  return triangles;
}

std::vector<Triangle> clipTriangles(const std::vector<Triangle> &triangles) {
  std::vector<Triangle> out;
  auto outside_clip_vol = [] (auto &v) {
    auto w = v.position.w;
    if (v.position.x > w || v.position.x < -w ||
        v.position.y > w || v.position.y < -w ||
        v.position.z > w || v.position.z < -w)
      return true;
    return false;
  };
  for (auto &tri : triangles) {
    if (std::any_of(std::cbegin(tri.v), std::cend(tri.v), outside_clip_vol))
      continue;
    out.emplace_back(tri);
  }
  return out;
}

std::vector<Triangle> cullBackFacing(const std::vector<Triangle> &triangles) {
  std::vector<Triangle> out;
  for (auto &tri : triangles) {
    auto darea = (tri.v[1].position.x - tri.v[0].position.x) *
                 (tri.v[2].position.y - tri.v[0].position.y) -
                 (tri.v[2].position.x - tri.v[0].position.x) *
                 (tri.v[1].position.y - tri.v[0].position.y);
    if (darea < 0)
      continue;
    out.push_back({{tri.v[0], tri.v[1], tri.v[2]}, darea});
  }
  return out;
}

void convertToScreenSpace(std::vector<Triangle> &triangles,
                          unsigned width, unsigned height) {
  for (auto &tri : triangles) {
    for (auto &vert : tri.v) {
      // To NDC.
      vert.position.x /= vert.position.w;
      vert.position.y /= vert.position.w;
      vert.position.z /= vert.position.w;
      vert.position.w = 1.f;

      // To screen space.
      vert.position.x = vert.position.x * (width - 1) / 2 + (width - 1) / 2;
      vert.position.y = vert.position.y * (height - 1) / 2 + (height - 1) / 2;
      vert.position.z = vert.position.z * .5f + .5f;
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
      rasterizeTriHalfSpace(tri, fragments);
    }
  }
  return fragments;
}

void invokeFragmentShader(const std::vector<Fragment> &fragments,
                          FrameBuffer &fb,
                          const Uniform &uniform,
                          FragmentShader shader) {
  for (auto &frag : fragments) {
    auto &depth = fb.getDepth(frag.frag_coord.x, frag.frag_coord.y);
    // Early z test.
    if (frag.frag_coord.z >= depth)
      continue;

    shader(frag, uniform, fb.getColor(frag.frag_coord.x, frag.frag_coord.y));
    depth = frag.frag_coord.z;
  }
}

} // namespace renderer
