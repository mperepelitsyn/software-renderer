#pragma once

#include <vector>
#include <string>

#include "renderer/pipeline.h"

namespace app {

std::vector<renderer::Vertex> parseObj(const std::string &path);

} // namespace app
