#pragma once

#include "lab2/common.h"

#include <string>

namespace lab2 {

struct Operator {
    Id id;
    Id branchId;
    std::string name;
    bool mayHandle(Id branch) const;
    std::string signature() const;
};

} // namespace lab2
