#pragma once

#include "lab2/common.h"

#include <string>

namespace lab2 {

struct IdentityDocument {
    std::string number;
    std::string issuer;
    Day expires;
    bool complete() const;
    bool validOn(Day day) const;
};

} // namespace lab2
