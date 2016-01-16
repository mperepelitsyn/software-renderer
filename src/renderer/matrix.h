#pragma once

#include "renderer/vector.h"

namespace renderer {

// Column major representation.
struct Mat4 {
  Mat4() {}
  Mat4(float d)
    : data{{d, 0.f, 0.f, 0.f},
           {0.f, d, 0.f, 0.f},
           {0.f, 0.f, d, 0.f},
           {0.f, 0.f, 0.f, d}} {}

  Mat4(const Vec3 &x, const Vec3 &y, const Vec3 &z, const Vec3 &w)
    : data{
        {x.x, y.x, z.x, w.x},
        {x.y, y.y, z.y, w.y},
        {x.z, y.z, z.z, w.z},
        {0.f, 0.f, 0.f, 1.f},
      } {};

  Mat4(const Vec4 &r1, const Vec4 &r2, const Vec4 &r3, const Vec4 &r4)
    : data {r1, r2, r3, r4 } {}

  Vec4 &operator[](unsigned idx) { return data[idx]; }
  const Vec4 &operator[](unsigned idx) const { return data[idx]; }

  Mat4 operator*(const Mat4 &m) const;
  Vec4 operator*(const Vec4 &v) const;

  Vec4 data[4];
};

Mat4 scale(float xf, float yf, float zf);
Mat4 translate(const Vec3 &dir);

// CCW rotations, angle is in radians.
Mat4 rotateX(float angle);
Mat4 rotateY(float angle);
Mat4 rotateZ(float angle);

// Right handed coordinate system.
Mat4 createPerspProjMatrix(float fovy, float aspect, float znear, float zfar);
Mat4 createViewMatrix(const Vec3 &pos, const Vec3 &target, const Vec3 &up);

constexpr inline long double operator""_deg(long double deg) {
  return 3.14159265358979f / 180.f * deg;
}

} // namespace renderer
