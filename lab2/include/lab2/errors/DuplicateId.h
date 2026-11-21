#pragma once

#include <stdexcept>

namespace lab2 {

class DuplicateId : public std::runtime_error {
    using runtime_error::runtime_error;
};

} // namespace lab2
