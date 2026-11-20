#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"

namespace lab2 {

struct HoldPolicy {
    int maximumAdvanceDays;
    int pickupGraceDays;
    bool allowSameDay;
    void checkRequest(const BookingRequest& request) const;
    bool permitsPickup(const Period& period, Day day) const;
    bool expired(const Period& period, Day day) const;
};

} // namespace lab2
