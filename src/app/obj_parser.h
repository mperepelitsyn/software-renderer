#pragma once

#include <vector>
#include <string>

#include "renderer/pipeline.h"

namespace app {

struct ObjAttrs : public renderer::Attrs {
  ObjAttrs(const renderer::Vec3 &normal, const renderer::Vec2 &tc)
    : normal{normal}, tc{tc} {}
  renderer::Vec3 normal;
  renderer::Vec2 tc;
};

std::vector<renderer::Vertex> parseObj(const std::string &path);

} // namespace app
