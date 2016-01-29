#pragma once

#include <cstring>
#include <type_traits>
#include <vector>

#include "renderer/vector.h"

namespace renderer {

struct UNorm {
  UNorm() = default;
  UNorm(unsigned rgba) : rgba{rgba} {}
  UNorm(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    : r{r}, g{g}, b{b}, a{a} {}

  union {
    struct { unsigned char r, g, b, a; };
    unsigned rgba;
  };
};

template<class T>
class Texture {
  using Type = std::conditional_t<std::is_same<T, UNorm>::value, Vec4, T>;

 public:
  Texture(unsigned width, unsigned height, const std::vector<T> &buf)
    : buffer_{buf}, width_{width}, height_{height} {}

  Texture(unsigned width, unsigned height)
    : buffer_(width * height), width_{width}, height_{height} {}

  Type sample(float u, float v) const
    { return fetchTexel(u * (width_ - 1), v * (height_ - 1)); }

  Type fetchTexel(unsigned x, unsigned y) const
    { return buffer_[y * width_ + x]; }

  void setTexel(unsigned x, unsigned y, const Type &texel)
    { buffer_[y * width_ + x] = texel; }

  void fill(const T &val)
    { std::fill(buffer_.begin(), buffer_.end(), val); }

  void clear()
    { std::memset(&buffer_[0], 0x0, getSize()); }

  size_t getSize() const { return width_ * height_ * sizeof(T); }
  unsigned getWidth() const { return width_; }
  unsigned getHeight() const { return height_; }
  const void *getRawBuffer() const { return &buffer_[0]; }

 private:
  std::vector<T> buffer_;
  unsigned width_;
  unsigned height_;
};

template<>
Vec4 Texture<UNorm>::fetchTexel(unsigned x, unsigned y) const;

template<>
void Texture<UNorm>::setTexel(unsigned x, unsigned y, const Vec4 &color);

template<>
void Texture<UNorm>::fill(const UNorm &val);

} // namespace renderer
