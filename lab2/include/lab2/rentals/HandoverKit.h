#pragma once

#include "lab2/common.h"
#include "lab2/rentals/Accessory.h"

#include <string>
#include <vector>

namespace lab2 {

struct HandoverKit {
    Id assetId;
    std::vector<Accessory> expected;
    std::vector<std::string> present;
    std::vector<std::string> missing() const;
    void verify(Id asset) const;
};

} // namespace lab2
