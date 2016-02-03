#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>

#include <immintrin.h>

#include "renderer/pipeline.h"

namespace renderer {

namespace {

struct Edge {
  int eq;
  int step_x;
  int step_y;
};

float lerp(float a, float b, float w) {
  return (1.f - w) * a + w * b;
}

Edge setup_edge(int x1, int y1, int x2, int y2, int x_start, int y_start,
                int prec) {
  auto dx = x2 - x1;
  auto dy = y2 - y1;
  auto bias = dy < 0 || (dy == 0 && dx < 0) ? 0 : -1;

  int e = (static_cast<long long>(dx) * (y_start - y2)
           - static_cast<long long>(dy) * (x_start - x2) + bias)
           >> prec;
  return {e, dy, dx};
}

void precomputeAttrs(const Triangle &tri, unsigned attr_count) {
  if (!attr_count)
    return;

  float *in[] = {reinterpret_cast<float*>(tri.v[0]->attr),
                 reinterpret_cast<float*>(tri.v[1]->attr),
                 reinterpret_cast<float*>(tri.v[2]->attr)};

  auto w0 = _mm256_broadcast_ss(&tri.v[0]->pos.w);
  auto w1 = _mm256_broadcast_ss(&tri.v[1]->pos.w);
  auto w2 = _mm256_broadcast_ss(&tri.v[2]->pos.w);

  auto vecs = (attr_count + 7) / 8;
  for (auto i = 0u; i < vecs; ++i) {
    auto in0 = _mm256_load_ps(in[0]);
    auto in1 = _mm256_load_ps(in[1]);
    auto in2 = _mm256_load_ps(in[2]);
    auto attr0 = _mm256_mul_ps(in0, w0);
    _mm256_store_ps(in[1], _mm256_sub_ps(_mm256_mul_ps(in1, w1), attr0));
    _mm256_store_ps(in[2], _mm256_sub_ps(_mm256_mul_ps(in2, w2), attr0));
    _mm256_store_ps(in[0], attr0);

    in[0] += 8;
    in[1] += 8;
    in[2] += 8;
  }
}

} // namespace

void Pipeline::draw() {
  assert(vb_);
  assert(prog_);

  vert_arena_.reset(vb_->count, sizeof(VertexH), alignof(VertexH));
  attr_arena_.reset(vb_->count, (prog_->attr_count + 7) / 8 * 32, 32);

  auto triangles = transform();
  rasterize(triangles);
}

std::vector<Triangle> Pipeline::transform() {
  auto buf = static_cast<const char*>(vb_->ptr);
  auto tri_count = vb_->count / 3;
  std::vector<Triangle> out{tri_count};

  auto tri_idx = 0u;
  for (auto i = 0u; i < tri_count; ++i) {
    auto clipped = 0u;

    // Assemble a triangle.
    auto &tri = out[tri_idx];
    for (auto v_idx = 0u; v_idx < 3; ++v_idx) {
      auto &v = tri.v[v_idx];

      v = vert_arena_.allocate<VertexH>();
      v->attr = attr_arena_.allocate<void>();
      prog_->vs(*reinterpret_cast<const Vertex*>(buf), uniform_, *v);

      // Clip trivially rejectable.
      if (v->pos.x > v->pos.w || v->pos.x < -v->pos.w ||
          v->pos.y > v->pos.w || v->pos.y < -v->pos.w ||
          v->pos.z > v->pos.w || v->pos.z < -v->pos.w)
        ++clipped;

      buf += vb_->stride;
    }

    if (clipped == 3)
      continue;

    for (auto vert : tri.v) {
      // To NDC.
      auto z_recipr = 1.f / vert->pos.w;
      vert->pos.x *= z_recipr;
      vert->pos.y *= z_recipr;
      vert->pos.z *= z_recipr;
      vert->pos.w = z_recipr;

      // To screen space.
      auto width = fb_->getWidth();
      auto height = fb_->getHeight();
      vert->pos.x = (vert->pos.x * (width - 1) + width - 1) * .5f;
      vert->pos.y = (vert->pos.y * (height - 1) + height - 1) * .5f;
      vert->pos.z = vert->pos.z * .5f + .5f;
    }
    ++tri_idx;
  }
  out.resize(tri_idx);

  return out;
}

void Pipeline::rasterize(std::vector<Triangle> &triangles) {
  for (auto &tri : triangles) {
    if (wireframe_) {
      // TODO: Deal with the duplication of the area calculation.
      auto area = (tri.v[1]->pos.x - tri.v[0]->pos.x) *
                  (tri.v[2]->pos.y - tri.v[0]->pos.y) -
                  (tri.v[2]->pos.x - tri.v[0]->pos.x) *
                  (tri.v[1]->pos.y - tri.v[0]->pos.y);
      if ((culling_ == BACK_FACING && area <= 0.f)
          || (culling_ == FRONT_FACING && area >= 0.f))
        continue;
      rasterizeLine(*tri.v[0], *tri.v[1]);
      rasterizeLine(*tri.v[0], *tri.v[2]);
      rasterizeLine(*tri.v[1], *tri.v[2]);
    }
    else {
      rasterizeTriHalfSpace(tri);
    }
  }
}

// Bresenham's line algorithm.
void Pipeline::rasterizeLine(const VertexH &v0, const VertexH &v1) {
  int x0 = v0.pos.x;
  int x1 = v1.pos.x;
  int y0 = v0.pos.y;
  int y1 = v1.pos.y;
  auto from = &v0;
  auto to = &v1;
  auto steep = false;
  if (std::abs(y1 - y0) > std::abs(x1 - x0)) {
    steep = true;
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(from, to);
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  int y = y0;
  int dx = x1 - x0;
  int dy = std::abs(y1 - y0);
  auto diff = 2 * dy - dx;
  int y_growth = y1 > y0 ? 1 : -1;
  auto w_step = 1.f / dx;
  auto w = 0.f;

  if (steep)
    fill(*from, *to, y0, x0, w);
  else
    fill(*from, *to, x0, y0, w);

  if (diff > 0) {
    y += y_growth;
    diff -= 2 * dx;
  }
  for (int x = x0 + 1; x <= x1; ++x) {
    w += w_step;
    if (steep)
      fill(*from, *to, y, x, w);
    else
      fill(*from, *to, x, y, w);

    diff += 2 * dy;
    if (diff > 0) {
      y += y_growth;
      diff -= 2 * dx;
    }
  }
}

// Top-left filling convention.
void Pipeline::rasterizeTriHalfSpace(Triangle &tri) {
  // 8 bit sub pixel precision.
  constexpr auto prec_bits = 8;
  constexpr auto step = 1 << prec_bits;
  constexpr auto mask = ~(step - 1);
  constexpr auto offset = (step - 1) >> 1;
  constexpr auto scale = static_cast<float>(step);

  int x0 = tri.v[0]->pos.x * scale;
  int x1 = tri.v[1]->pos.x * scale;
  int x2 = tri.v[2]->pos.x * scale;
  int y0 = tri.v[0]->pos.y * scale;
  int y1 = tri.v[1]->pos.y * scale;
  int y2 = tri.v[2]->pos.y * scale;

  // Culling and degenerate triangle handling.
  int area = (static_cast<long long>(x1 - x0) * (y2 - y0)
              - static_cast<long long>(y1 - y0) * (x2 - x0)) >> prec_bits;

  auto reverse_winding = [&]() {
      std::swap(tri.v[1], tri.v[2]);
      std::swap(x1, x2);
      std::swap(y1, y2);
      area = -area;
  };

  switch (culling_) {
  case NONE:
    if (area < 0)
      reverse_winding();
    break;
  case BACK_FACING:
    if (area <= 0)
      return;
    break;
  case FRONT_FACING:
    if (area < 0) {
      reverse_winding();
      break;
    }
    return;
  }
  if (area == 0)
    return;

  auto area_rec = 1.f / area;

  auto aabb_x = std::minmax({x0, x1, x2});
  auto aabb_y = std::minmax({y0, y1, y2});
  aabb_x = {std::max(0, aabb_x.first),
            std::min((int)(fb_->getWidth() - 1) << prec_bits, aabb_x.second)};
  aabb_y = {std::max(0, aabb_y.first),
            std::min((int)(fb_->getHeight() - 1) << prec_bits, aabb_y.second)};

  auto x_start = (aabb_x.first & mask) + offset;
  auto y_start = (aabb_y.first & mask) + offset;
  auto x_end = (aabb_x.second & mask) + offset;
  auto y_end = (aabb_y.second & mask) + offset;

  auto edge0 = setup_edge(x1, y1, x2, y2, x_start, y_start, prec_bits);
  auto edge1 = setup_edge(x2, y2, x0, y0, x_start, y_start, prec_bits);
  auto edge2 = setup_edge(x0, y0, x1, y1, x_start, y_start, prec_bits);

  precomputeAttrs(tri, prog_->attr_count);

  x_end >>= prec_bits;
  y_end >>= prec_bits;
  for (auto y = y_start >> prec_bits; y <= y_end; ++y) {
    auto e0 = edge0.eq;
    auto e1 = edge1.eq;
    auto e2 = edge2.eq;

    for (auto x = x_start >> prec_bits; x <= x_end; ++x) {
      if ((e0 | e1 | e2) >= 0) {
        auto w0 = e0 * area_rec;
        auto w1 = e1 * area_rec;
        auto w2 = 1 - w0 - w1;

        fill(tri, x, y, w0, w1, w2);
      }
      e0 -= edge0.step_x;
      e1 -= edge1.step_x;
      e2 -= edge2.step_x;
    }

    edge0.eq += edge0.step_y;
    edge1.eq += edge1.step_y;
    edge2.eq += edge2.step_y;
  }
}

void Pipeline::fill(const VertexH &v1, const VertexH &v2,
                    float x, float y, float w) {
  auto z_s = lerp(v1.pos.z, v2.pos.z, w);

  // Early Z-test.
  if (z_s >= fb_->getDepth(x, y))
    return;

  auto z_v = lerp(v1.pos.w, v2.pos.w, w);

  Fragment frag;
  float storage[max_attr_size];
  frag.attr = &storage;
  const float *in[] = {reinterpret_cast<const float*>(&v1.attr),
                       reinterpret_cast<const float*>(&v2.attr)};
  frag.coord.x = x;
  frag.coord.y = y;
  frag.coord.z = z_s;

  for (auto i = 0u; i < prog_->attr_count; ++i) {
    storage[i] = lerp(*(in[0] + i) * v1.pos.w, *(in[1] + i) * v2.pos.w, w) / z_v;
  }
  invokeFragmentShader(frag);
}

void Pipeline::fill(const Triangle &tri, float x, float y,
                    float w0, float w1, float w2) {
  auto z_s = w0 * tri.v[0]->pos.z + w1 * tri.v[1]->pos.z + w2 * tri.v[2]->pos.z;

  // Early Z-test.
  if (z_s >= fb_->getDepth(x, y))
    return;

  Fragment frag;
  alignas(32) float storage[max_attr_size];
  frag.attr = &storage;

  frag.coord.x = x;
  frag.coord.y = y;
  frag.coord.z = z_s;

  // Interpolate attributes.
  if (prog_->attr_count) {
    const float *in[] = {reinterpret_cast<float*>(tri.v[0]->attr),
                         reinterpret_cast<float*>(tri.v[1]->attr),
                         reinterpret_cast<float*>(tri.v[2]->attr)};
    auto z_v_rec = 1.f /
         (w0 * tri.v[0]->pos.w + w1 * tri.v[1]->pos.w + w2 * tri.v[2]->pos.w);

    auto vw1 = _mm256_broadcast_ss(&w1);
    auto vw2 = _mm256_broadcast_ss(&w2);
    auto vz_rec = _mm256_broadcast_ss(&z_v_rec);

    auto vecs = (prog_->attr_count + 7) / 8;
    for (auto i = 0u; i < vecs; ++i) {
      auto in0 = _mm256_load_ps(in[0] + i * 8);
      auto in1 = _mm256_load_ps(in[1] + i * 8);
      auto in2 = _mm256_load_ps(in[2] + i * 8);
      _mm256_store_ps(&storage[i * 8], _mm256_mul_ps(_mm256_add_ps(in0,
              _mm256_add_ps(_mm256_mul_ps(in1, vw1), _mm256_mul_ps(in2, vw2))),
              vz_rec));
    }
  }

  invokeFragmentShader(frag);
}

void Pipeline::invokeFragmentShader(const Fragment &frag) {
  Vec4 color;
  prog_->fs(frag, uniform_, color);
  fb_->setPixel(frag.coord.x, frag.coord.y, color, frag.coord.z);
}

} // namespace renderer
