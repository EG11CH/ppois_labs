#include "lab2/inventory/Branch.h"
#include "lab2/common.h"

#include <string>

namespace lab2 {

bool Branch::acceptsAt(Id assetBranch) const {
    return acceptsReturns && id == assetBranch;
}

std::string Branch::label() const {
    return name + " — " + address;
}

} // namespace lab2
