#include <cmath>

#include "matrix.h"

Mat4 Mat4::operator*(const Mat4 &m) const {
  Mat4 r{0.f};
  for (auto i = 0u; i < 4; ++i) {
    for (auto j = 0u; j < 4; ++j) {
      r[i][j] = 0;
      for (auto k = 0u; k < 4; ++k)
        r[i][j] += data[i][k] * m[k][j];
    }
  }
  return r;
};

Vec4 Mat4::operator*(const Vec4 &v) const {
  return {dot(data[0], v), dot(data[1], v), dot(data[2], v), dot(data[3], v)};
}

Mat4 scale(float xf, float yf, float zf) {
  return {{xf, 0.f, 0.f}, {0.f, yf, 0.f}, {0.f, 0.f, zf}, {0.f, 0.f, 0.f}};
}

Mat4 translate(const Vec3 &dir) {
  return {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}, dir};
}

Mat4 rotateX(float angle) {
  return {
    {1.f, 0.f, 0.f},
    {0.f, std::cos(angle), std::sin(angle)},
    {0.f, -std::sin(angle), std::cos(angle)},
    {0.f, 0.f, 0.f}
  };
}

Mat4 rotateY(float angle) {
  return {
    {std::cos(angle), 0.f, std::sin(angle)},
    {0.f, 1.f, 0.f},
    {-std::sin(angle), 0.f, std::cos(angle)},
    {0.f, 0.f, 0.f}
  };
}

Mat4 rotateZ(float angle) {
  return {
    {std::cos(angle), std::sin(angle), 0.f},
    {-std::sin(angle), std::cos(angle), 0.f},
    {0.f, 0.f, 1.f},
    {0.f, 0.f, 0.f}
  };
}

Mat4 createPerspProjMatrix(float fovy, float aspect, float znear, float zfar) {
  auto t = std::tan(fovy / 2);
  auto r = aspect * t;

  return {
    {1 / r, 0.f, 0.f, 0.f},
    {0.f, 1 / t, 0.f, 0.f},
    {0.f, 0.f, -(zfar + znear) / (zfar - znear),
               -2.f * znear * zfar / (zfar - znear)},
    {0.f, 0.f, -1.f, 0.f},
  };
}

Mat4 createViewMatrix(const Vec3 &pos, const Vec3 &target, const Vec3 &up) {
  auto w = normalize(pos - target);
  auto u = cross(up, w);
  auto v = cross(w, u);

  return {
    {u.x, u.y, u.z, -pos.x},
    {v.x, v.y, v.z, -pos.y},
    {w.x, w.y, w.z, -pos.z},
    {0.f, 0.f, 0.f, 1.f},
  };
}
