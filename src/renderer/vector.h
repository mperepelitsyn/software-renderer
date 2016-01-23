#pragma once

#include <cmath>

namespace renderer {

struct Vec2 {
  Vec2() {}
  Vec2(float a, float b) : x{a}, y{b} {}

  float &operator[](unsigned i) { return data[i]; }
  const float &operator[](unsigned i) const { return data[i]; }
  Vec2 operator+(const Vec2 &v) const { return {x + v.x, y + v.y}; }
  Vec2 operator-(const Vec2 &v) const { return {x - v.x, y - v.y}; }
  Vec2 operator/(float d) const { return {x / d, y / d}; }
  Vec2 operator*(float m) const { return {x * m, y * m}; }
  Vec2 operator*(const Vec2 &v) const { return {x * v.x, y * v.y}; }

  union {
    struct {
      float r;
      float g;
    };
    struct {
      float x;
      float y;
    };
    float data[2];
  };
};

struct Vec3 {
  Vec3() {}
  Vec3(float a, float b, float c) : x{a}, y{b}, z{c} {}

  float &operator[](unsigned i) { return data[i]; }
  const float &operator[](unsigned i) const { return data[i]; }
  Vec3 operator+(const Vec3 &v) const { return {x + v.x, y + v.y, z + v.z}; }
  Vec3 operator-(const Vec3 &v) const { return {x - v.x, y - v.y, z - v.z}; }
  Vec3 operator/(float d) const { return {x / d, y / d, z / d}; }
  Vec3 operator*(float m) const { return {x * m, y * m, z * m}; }
  Vec3 operator*(const Vec3 &v) const { return {x * v.x, y * v.y, z * v.z}; }

  union {
    struct {
      float r;
      float g;
      float b;
    };
    struct {
      float x;
      float y;
      float z;
    };
    float data[3];
  };
};

struct Vec4 {
  Vec4() {}
  Vec4(float a, float b, float c, float d) : x{a}, y{b}, z{c}, w{d} {}
  Vec4(const Vec3 &v, float w) : x{v.x}, y{v.y}, z{v.z}, w{w} {}

  float &operator[](unsigned i) { return data[i]; }
  const float &operator[](unsigned i) const { return data[i]; }
  Vec4 operator+(const Vec4 &v) const
    { return {x + v.x, y + v.y, z + v.z, w + v.w}; }
  Vec4 operator-(const Vec4 &v) const
    { return {x - v.x, y - v.y, z - v.z, w - v.w}; }
  Vec4 operator/(float d) const { return {x / d, y / d, z / d, w / d}; }
  Vec4 operator*(float m) const { return {x * m, y * m, z * m, w * m}; }
  Vec4 operator*(const Vec4 &v) const
    { return {x * v.x, y * v.y, z * v.z, w * v.w}; }

  union {
    struct {
      float r;
      float g;
      float b;
      float a;
    };
    struct {
      float x;
      float y;
      float z;
      float w;
    };
    float data[4];
  };
};

inline float dot(const Vec3 &u, const Vec3 &v) {
  return u.x * v.x + u.y * v.y + u.z * v.z;
}

inline float dot(const Vec4 &u, const Vec4 &v) {
  return u.x * v.x + u.y * v.y + u.z * v.z + u.w * v.w;
}

inline Vec3 cross(const Vec3 &u, const Vec3 &v) {
  return {u.y * v.z - u.z * v.y, u.z * v.x - u.x * v.z, u.x * v.y - u.y * v.x};
}

inline float length(const Vec3 &v) {
  return std::sqrt(std::pow(v.x, 2) + std::pow(v.y, 2) + std::pow(v.z, 2));
}

inline Vec3 reflect(const Vec3 &v, const Vec3 &n) {
  return v - n * dot(n, v) * 2;
}

inline Vec3 normalize(const Vec3 &v) {
  auto len = length(v);
  return len == 0.f ?  Vec3{0.f, 0.f, 0.f} : v / len;
}

} // namespace renderer
