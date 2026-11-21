#include "lab2/rentals/Accessory.h"

#include <algorithm>
#include <string>
#include <vector>

namespace lab2 {

bool Accessory::missingFrom(const std::vector<std::string>& present) const {
    return required && std::find(present.begin(), present.end(), code) == present.end();
}

std::string Accessory::description() const {
    return label + " (" + code + ")";
}

} // namespace lab2
