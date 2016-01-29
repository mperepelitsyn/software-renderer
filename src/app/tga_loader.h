#pragma once

#include <string>
#include <vector>

#include "renderer/texture.h"

namespace app {

std::vector<renderer::UNorm> loadTGA(const std::string &path);

} // namespace app
