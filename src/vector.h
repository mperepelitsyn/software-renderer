#pragma once

struct Vec2 {
  Vec2() {}
  Vec2(float a, float b) : x{a}, y{b} {}

  union {
    struct {
      float r;
      float g;
    };
    struct {
      float x;
      float y;
    };
  };
};

struct Vec3 {
  Vec3() {}
  Vec3(float a, float b, float c) : x{a}, y{b}, z{c} {}

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
  };
};

struct Vec4 {
  Vec4() {}
  Vec4(float a, float b, float c, float d) : x{a}, y{b}, z{c}, w{d} {}

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
  };
};
