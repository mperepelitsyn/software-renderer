#include <algorithm>
#include <cstdlib>
#include <functional>

#include "renderer/pipeline.h"

namespace renderer {

namespace {

float getNearestPixelCenter(float x) {
  return floor(x) + 0.5;
}

float lerp(float a, float b, float w) {
  return (1.f - w) * a + w * b;
}

void precomputeAttrs(const Triangle &tri, unsigned attr_count) {
  float *in[] = {reinterpret_cast<float*>(tri.v[0] + 1),
                 reinterpret_cast<float*>(tri.v[1] + 1),
                 reinterpret_cast<float*>(tri.v[2] + 1)};
  for (auto i = 0u; i < attr_count; ++i) {
    *(in[0] + i) *= tri.v[0]->pos.w;
    *(in[1] + i) = *(in[1] + i) * tri.v[1]->pos.w - *(in[0] + i);
    *(in[2] + i) = *(in[2] + i) * tri.v[2]->pos.w - *(in[0] + i);
  }
}

} // namespace

void Pipeline::draw() {
  assert(vb_);
  assert(prog_);

  arena_.reset(vb_->count,
               prog_->attr_count * sizeof(float) + sizeof(VertexH::pos));

  auto transformed = invokeVertexShader();
  auto triangles = assembleTriangles(transformed);
  triangles = clipTriangles(triangles);
  convertToScreenSpace(triangles, fb_->getWidth(), fb_->getHeight());
  triangles = cullTriangles(triangles);
  rasterize(triangles);
}

std::vector<VertexH*> Pipeline::invokeVertexShader() {
  std::vector<VertexH*> transformed{vb_->count};
  auto buf = static_cast<const char*>(vb_->ptr);

  for (auto i = 0u; i < vb_->count; ++i) {
    transformed[i] = arena_.allocate<VertexH>();
    prog_->vs(*reinterpret_cast<const Vertex*>(buf), uniform_, *transformed[i]);
    buf += vb_->stride;
  }

  return transformed;
}

std::vector<Triangle> Pipeline::assembleTriangles(
    const std::vector<VertexH*> &vertices) {
  auto tri_count = vertices.size() / 3;
  std::vector<Triangle> triangles{tri_count};

  for (auto i = 0u; i < tri_count; ++i) {
    triangles[i].v[0] = vertices[i * 3];
    triangles[i].v[1] = vertices[i * 3 + 1];
    triangles[i].v[2] = vertices[i * 3 + 2];
  }

  return triangles;
}

std::vector<Triangle> Pipeline::clipTriangles(
    const std::vector<Triangle> &triangles) {
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
    if (std::all_of(std::cbegin(tri.v), std::cend(tri.v), outside_clip_vol))
      continue;
    out.emplace_back(tri);
  }
  return out;
}

std::vector<Triangle> Pipeline::cullTriangles(
    const std::vector<Triangle> &triangles) {
  std::vector<Triangle> out;

  for (auto &tri : triangles) {
    auto darea = (tri.v[1]->pos.x - tri.v[0]->pos.x) *
                 (tri.v[2]->pos.y - tri.v[0]->pos.y) -
                 (tri.v[2]->pos.x - tri.v[0]->pos.x) *
                 (tri.v[1]->pos.y - tri.v[0]->pos.y);
    switch (culling_) {
    case NONE:
      if (darea < 0)
        out.push_back({{tri.v[0], tri.v[2], tri.v[1]}, -darea});
      else
        out.push_back({{tri.v[0], tri.v[1], tri.v[2]}, darea});
      break;
    case BACK_FACING:
      if (darea >= 0)
        out.push_back({{tri.v[0], tri.v[1], tri.v[2]}, darea});
      break;
    case FRONT_FACING:
      if (darea < 0)
        out.push_back({{tri.v[0], tri.v[2], tri.v[1]}, -darea});
      break;
    }
  }

  return out;
}

void Pipeline::convertToScreenSpace(std::vector<Triangle> &triangles,
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

void Pipeline::rasterize(const std::vector<Triangle> &triangles) {
  for (auto &tri : triangles) {
    if (wireframe_) {
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
void Pipeline::rasterizeTriHalfSpace(const Triangle &tri) {
  auto x0 = tri.v[0]->pos.x;
  auto x1 = tri.v[1]->pos.x;
  auto x2 = tri.v[2]->pos.x;
  auto y0 = tri.v[0]->pos.y;
  auto y1 = tri.v[1]->pos.y;
  auto y2 = tri.v[2]->pos.y;

  auto aabb_x = std::minmax({x0, x1, x2});
  auto aabb_y = std::minmax({y0, y1, y2});
  aabb_x = {std::max(0.f, aabb_x.first),
            std::min(static_cast<float>(fb_->getWidth() - 1), aabb_x.second)};
  aabb_y = {std::max(0.f, aabb_y.first),
            std::min(static_cast<float>(fb_->getHeight() - 1), aabb_y.second)};

  auto e0_top_left = y1 > y2 || (y1 == y2 && x1 > x2);
  auto e1_top_left = y2 > y0 || (y2 == y0 && x2 > x0);
  auto e2_top_left = y0 > y1 || (y0 == y1 && x0 > x1);

  auto dx0 = x2 - x1;
  auto dx1 = x0 - x2;
  auto dx2 = x1 - x0;
  auto dy0 = y2 - y1;
  auto dy1 = y0 - y2;
  auto dy2 = y1 - y0;

  auto x_start = getNearestPixelCenter(aabb_x.first);
  auto y_start = getNearestPixelCenter(aabb_y.first);
  auto x_end = getNearestPixelCenter(aabb_x.second);
  auto y_end = getNearestPixelCenter(aabb_y.second);

  auto y_e0 =  dx0 * y_start - dy0 * x_start + dy0 * x1 - dx0 * y1;
  auto y_e1 =  dx1 * y_start - dy1 * x_start + dy1 * x2 - dx1 * y2;
  auto y_e2 =  dx2 * y_start - dy2 * x_start + dy2 * x0 - dx2 * y0;

  precomputeAttrs(tri, prog_->attr_count);

  for (auto y = y_start; y <= y_end; ++y) {
    auto e0 = y_e0;
    auto e1 = y_e1;
    auto e2 = y_e2;

    for (auto x = x_start; x <= x_end; ++x) {
      if ((e0 > 0 || (e0 == 0 && e0_top_left)) &&
          (e1 > 0 || (e1 == 0 && e1_top_left)) &&
          (e2 > 0 || (e2 == 0 && e2_top_left))) {
        auto w0 = e0 / tri.darea;
        auto w1 = e1 / tri.darea;
        auto w2 = 1 - w0 - w1;

        fill(tri, x, y, w0, w1, w2);
      }
      e0 -= dy0;
      e1 -= dy1;
      e2 -= dy2;
    }

    y_e0 += dx0;
    y_e1 += dx1;
    y_e2 += dx2;
  }
}

void Pipeline::fill(const VertexH &v1, const VertexH &v2,
                    float x, float y, float w) {
  float storage[max_fragment_size];
  auto frag = reinterpret_cast<Fragment*>(&storage);
  interpolate(v1, v2, x, y, w, frag);
  invokeFragmentShader(*frag);
}

void Pipeline::fill(const Triangle &tri, float x, float y,
                    float w0, float w1, float w2) {
  float storage[max_fragment_size];
  auto frag = reinterpret_cast<Fragment*>(&storage);
  interpolate(tri, x, y, w0, w1, w2, frag);
  invokeFragmentShader(*frag);
}

void Pipeline::interpolate(const VertexH &v1, const VertexH &v2,
                           float x, float y, float w, Fragment *frag) {
  auto z_s = lerp(v1.pos.z, v2.pos.z, w);
  auto z_v = lerp(v1.pos.w, v2.pos.w, w);

  const float *in[] = {reinterpret_cast<const float*>(&v1 + 1),
                       reinterpret_cast<const float*>(&v2 + 1)};
  frag->coord.x = x;
  frag->coord.y = y;
  frag->coord.z = z_s;
  frag += 1;
  auto out = reinterpret_cast<float*>(frag);

  for (auto i = 0u; i < prog_->attr_count; ++i) {
    out[i] = lerp(*(in[0] + i) * v1.pos.w, *(in[1] + i) * v2.pos.w, w) / z_v;
  }
}

void Pipeline::interpolate(const Triangle &tri, float x, float y,
                           float w0, float w1, float w2, Fragment *frag) {
  auto z_s = w0 * tri.v[0]->pos.z + w1 * tri.v[1]->pos.z + w2 * tri.v[2]->pos.z;
  auto z_v = w0 * tri.v[0]->pos.w + w1 * tri.v[1]->pos.w + w2 * tri.v[2]->pos.w;

  const float *in[] = {reinterpret_cast<float*>(tri.v[0] + 1),
                       reinterpret_cast<float*>(tri.v[1] + 1),
                       reinterpret_cast<float*>(tri.v[2] + 1)};
  frag->coord.x = x;
  frag->coord.y = y;
  frag->coord.z = z_s;
  frag += 1;
  auto out = reinterpret_cast<float*>(frag);

  for (auto i = 0u; i < prog_->attr_count; ++i) {
    out[i] = (*(in[0] + i) + *(in[1] + i) * w1 + *(in[2] + i) * w2) / z_v;
  }
}

void Pipeline::invokeFragmentShader(const Fragment &frag) {
  auto &depth = fb_->getDepth(frag.coord.x, frag.coord.y);
  // Early z test.
  if (frag.coord.z >= depth)
    return;

  prog_->fs(frag, uniform_, fb_->getColor(frag.coord.x, frag.coord.y));
  depth = frag.coord.z;
}

} // namespace renderer
