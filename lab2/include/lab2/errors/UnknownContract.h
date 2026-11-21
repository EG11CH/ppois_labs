#pragma once

#include <stdexcept>

namespace lab2 {

class UnknownContract : public std::runtime_error {
    using runtime_error::runtime_error;
};

} // namespace lab2
