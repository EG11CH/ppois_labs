#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"

namespace lab2 {

struct WaitRequest {
    Id id;
    BookingRequest request;
    bool fulfilled;
    bool matches(Id asset, Day day) const;
    BookingRequest reconsider(Day day) const;
};

} // namespace lab2
