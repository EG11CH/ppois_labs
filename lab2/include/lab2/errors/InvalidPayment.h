#pragma once

#include <stdexcept>

namespace lab2 {

class InvalidPayment : public std::runtime_error {
    using runtime_error::runtime_error;
};

} // namespace lab2
