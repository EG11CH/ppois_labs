#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"

namespace lab2 {

struct Availability {
    Id assetId;
    Period period;
    bool physicallyReady;
    bool booked;
    bool canReserve() const;
    void requireAvailable() const;
};

} // namespace lab2
