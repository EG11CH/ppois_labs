#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"

#include <string>

namespace lab2 {

struct BookingRequest {
    Id customerId;
    Id assetId;
    Day requestedOn;
    Period period;
    std::string voucher;
    void validate() const;
    bool concerns(Id asset, const Period& interval) const;
};

} // namespace lab2
