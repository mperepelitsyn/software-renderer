#include <algorithm>
#include <cstdlib>
#include <functional>

#include "renderer/pipeline.h"

namespace renderer {

namespace {

// Bresenham's line algorithm.
void rasterizeLine(std::vector<std::unique_ptr<const Fragment>> &frags,
                   const VertexH &v0,
                   const VertexH &v1) {
  int x0 = v0.pos.x;
  int x1 = v1.pos.x;
  int y0 = v0.pos.y;
  int y1 = v1.pos.y;
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
    frags.emplace_back(std::make_unique<Fragment>(Vec3(y0, x0, v0.pos.z)));
  else
    frags.emplace_back(std::make_unique<Fragment>(Vec3(x0, y0, v0.pos.z)));

  if (diff > 0) {
    y += y_growth;
    diff -= 2 * dx;
  }
  for (int x = x0 + 1; x <= x1; ++x) {
    // TODO: Interpolate later.
    if (steep)
      frags.emplace_back(std::make_unique<Fragment>(Vec3(y0, x0, v0.pos.z)));
    else
      frags.emplace_back(std::make_unique<Fragment>(Vec3(x0, y0, v0.pos.z)));

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

std::unique_ptr<const Fragment> interpolate(const Triangle &tri,
    float x, float y,
    float w0, float w1, float w2, unsigned attr_count) {
  constexpr auto v_offset = sizeof(VertexH::pos) / sizeof(float);
  constexpr auto f_offset = sizeof(Fragment::coord) / sizeof(float);
  auto z_s = w0 * tri.v[0]->pos.z + w1 * tri.v[1]->pos.z + w2 * tri.v[2]->pos.z;
  auto z_v = w0 * tri.v[0]->pos.w + w1 * tri.v[1]->pos.w + w2 * tri.v[2]->pos.w;

  const float *in[] = {reinterpret_cast<const float*>(tri.v[0]) + v_offset,
                       reinterpret_cast<const float*>(tri.v[1]) + v_offset,
                       reinterpret_cast<const float*>(tri.v[2]) + v_offset};
  auto out = std::make_unique<float[]>(f_offset + attr_count);

  out[0] = x;
  out[1] = y;
  out[2] = z_s;
  for (auto i = 0u; i < attr_count; ++i) {
    out[i + f_offset] = (*(in[0] + i) * w0 * tri.v[0]->pos.w +
                         *(in[1] + i) * w1 * tri.v[1]->pos.w +
                         *(in[2] + i) * w2 * tri.v[2]->pos.w) / z_v;
  }
  return std::unique_ptr<const Fragment>(
      reinterpret_cast<Fragment*>(out.release()));
}

// Top-left filling convention.
void rasterizeTriHalfSpace(const Triangle &tri, unsigned attr_count,
    std::vector<std::unique_ptr<const Fragment>> &frags) {
  auto x0 = tri.v[0]->pos.x;
  auto x1 = tri.v[1]->pos.x;
  auto x2 = tri.v[2]->pos.x;
  auto y0 = tri.v[0]->pos.y;
  auto y1 = tri.v[1]->pos.y;
  auto y2 = tri.v[2]->pos.y;
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

        frags.emplace_back(interpolate(tri, x, y, w0, w1, w2, attr_count));
      }
    }
  }
}

} // namespace

std::vector<VertexH*> invokeVertexShader(const VertexBuffer &vb,
    const Program &prog, const void *uniform, Arena &arena) {
  std::vector<VertexH*> transformed{vb.count};
  auto buf = static_cast<const char*>(vb.ptr);

  for (auto i = 0u; i < vb.count; ++i) {
    transformed[i] = arena.allocate<VertexH>();
    prog.vs(*reinterpret_cast<const Vertex*>(buf), uniform, *transformed[i]);
    buf += vb.stride;
  }

  return transformed;
}

std::vector<Triangle> assembleTriangles(const std::vector<VertexH*> &vertices) {
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
  auto outside_clip_vol = [] (auto v) {
    auto w = v->pos.w;
    if (v->pos.x > w || v->pos.x < -w ||
        v->pos.y > w || v->pos.y < -w ||
        v->pos.z > w || v->pos.z < -w)
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
    auto darea = (tri.v[1]->pos.x - tri.v[0]->pos.x) *
                 (tri.v[2]->pos.y - tri.v[0]->pos.y) -
                 (tri.v[2]->pos.x - tri.v[0]->pos.x) *
                 (tri.v[1]->pos.y - tri.v[0]->pos.y);
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
      auto z_recipr = 1.f / vert->pos.w;
      vert->pos.x *= z_recipr;
      vert->pos.y *= z_recipr;
      vert->pos.z *= z_recipr;
      vert->pos.w = z_recipr;

      // To screen space.
      vert->pos.x = vert->pos.x * (width - 1) / 2 + (width - 1) / 2;
      vert->pos.y = vert->pos.y * (height - 1) / 2 + (height - 1) / 2;
      vert->pos.z = vert->pos.z * .5f + .5f;
    }
  }
}

std::vector<std::unique_ptr<const Fragment>> rasterize(
    const std::vector<Triangle> &triangles, unsigned attr_count,
    bool wireframe) {
  std::vector<std::unique_ptr<const Fragment>> fragments;

  for (auto &tri : triangles) {
    if (wireframe) {
      rasterizeLine(fragments, *tri.v[0], *tri.v[1]);
      rasterizeLine(fragments, *tri.v[0], *tri.v[2]);
      rasterizeLine(fragments, *tri.v[1], *tri.v[2]);
    }
    else {
      rasterizeTriHalfSpace(tri, attr_count, fragments);
    }
  }

  return fragments;
}

void invokeFragmentShader(
    const std::vector<std::unique_ptr<const Fragment>> &fragments,
    FrameBuffer &fb, const void *uniform, FragmentShader shader) {
  for (auto &frag : fragments) {
    auto &depth = fb.getDepth(frag->coord.x, frag->coord.y);
    // Early z test.
    if (frag->coord.z >= depth)
      continue;

    shader(*frag.get(), uniform, fb.getColor(frag->coord.x, frag->coord.y));
    depth = frag->coord.z;
  }
}

} // namespace renderer
