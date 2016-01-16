#pragma once

#include <stdexcept>

namespace app {

class Error : public std::runtime_error {
  using runtime_error::runtime_error;
};

} // namespace app
