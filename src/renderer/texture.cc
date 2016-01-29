#include "renderer/texture.h"

namespace {

inline float clamp(float v, float min, float max) {
  auto t = v < min ? min : v;
  return t > max ? max : t;
}

} // namespace

namespace renderer {

template<>
Vec4 Texture<UNorm>::fetchTexel(unsigned x, unsigned y) const {
  constexpr auto div = 1.f / 255.f;
  auto &t = buffer_[y * width_ + x];
  return {t.r * div, t.g * div, t.b * div, t.a * div};
}

template<>
void Texture<UNorm>::setTexel(unsigned x, unsigned y, const Vec4 &color) {
  buffer_[y * width_ + x] = {
    static_cast<unsigned char>(clamp(color.r, 0.f, 1.f) * 255.f),
    static_cast<unsigned char>(clamp(color.g, 0.f, 1.f) * 255.f),
    static_cast<unsigned char>(clamp(color.b, 0.f, 1.f) * 255.f),
    255
  };
}

template<>
void Texture<UNorm>::fill(const UNorm &val) {
  std::memset(&buffer_[0], val.rgba, getSize());
}

} // namespace renderer
