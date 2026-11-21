#pragma once

#include "lab2/common.h"

#include <string>

namespace lab2 {

struct Branch {
    Id id;
    std::string name;
    std::string address;
    bool acceptsReturns;
    bool acceptsAt(Id assetBranch) const;
    std::string label() const;
};

} // namespace lab2
