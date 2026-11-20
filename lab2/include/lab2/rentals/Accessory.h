#pragma once

#include <string>
#include <vector>

namespace lab2 {

struct Accessory {
    std::string code;
    std::string label;
    bool required;
    bool missingFrom(const std::vector<std::string>& present) const;
    std::string description() const;
};

} // namespace lab2
