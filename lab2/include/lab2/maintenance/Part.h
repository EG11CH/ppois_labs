#pragma once

#include "lab2/common.h"

#include <string>

namespace lab2 {

struct Part {
    std::string sku;
    std::string name;
    std::string supplier;
    Money unitPrice;
    void validate() const;
    Money priceFor(int quantity) const;
};

} // namespace lab2
