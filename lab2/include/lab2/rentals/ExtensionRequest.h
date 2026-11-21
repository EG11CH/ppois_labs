#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"

#include <string>

namespace lab2 {

struct ExtensionRequest {
    Id contractId;
    Day requestedOn;
    Day until;
    std::string reason;
    Period applyTo(Id contract, Day issued, const Period& current) const;
};

} // namespace lab2
