#pragma once

#include <string>
#include <vector>

#include "renderer/vector.h"

namespace app {

std::vector<renderer::Vec3> loadTGA(const std::string &path);

} // namespace app
