#pragma once

#include "lab2/common.h"
#include "lab2/maintenance/Part.h"

#include <string>

namespace lab2 {

struct PartUsage {
    Part part;
    int quantity;
    std::string reason;
    Money cost() const;
    std::string summary() const;
};

} // namespace lab2
