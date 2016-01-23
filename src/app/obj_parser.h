#pragma once

#include <vector>
#include <string>

#include "renderer/pipeline.h"

namespace app {

struct ObjVertex : renderer::Vertex {
  ObjVertex(const renderer::Vec3 &pos, const renderer::Vec3 &normal,
            const renderer::Vec2 &tc)
    : Vertex{pos}, normal{normal}, tc{tc} {}

  renderer::Vec3 normal;
  renderer::Vec2 tc;
};

std::vector<ObjVertex> parseObj(const std::string &path);

} // namespace app
