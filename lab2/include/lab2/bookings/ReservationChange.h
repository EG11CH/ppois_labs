#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"

#include <string>

namespace lab2 {

struct ReservationChange {
    Id reservationId;
    Day day;
    Period replacement;
    std::string reason;
    void verify(Id id, Day bookedOn) const;
    BookingRequest applyTo(const BookingRequest& original) const;
};

} // namespace lab2
