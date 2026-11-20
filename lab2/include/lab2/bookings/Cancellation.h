#pragma once

#include "lab2/bookings/HoldPolicy.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"

#include <string>

namespace lab2 {

struct Cancellation {
    Id reservationId;
    Day day;
    std::string reason;
    void verify(Id id, Day bookedOn) const;
    bool isNoShow(const HoldPolicy& policy, const Period& period) const;
};

} // namespace lab2
