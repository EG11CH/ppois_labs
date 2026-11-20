#include "lab2/rentals/HandoverKit.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidInspection.h"

#include <string>
#include <vector>

namespace lab2 {

std::vector<std::string> HandoverKit::missing() const {
    std::vector<std::string> result;
    for (const auto& accessory : expected) {
        if (accessory.missingFrom(present)) {
            result.push_back(accessory.description());
        }
    }
    return result;
}

void HandoverKit::verify(Id asset) const {
    if (assetId != asset || !missing().empty()) {
        throw InvalidInspection("Укомплектуй предмет перед выдачей.");
    }
}

} // namespace lab2
