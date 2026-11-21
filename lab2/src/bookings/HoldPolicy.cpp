#include "lab2/bookings/HoldPolicy.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidPeriod.h"

#include <chrono>

namespace lab2 {

void HoldPolicy::checkRequest(const BookingRequest& request) const {
    request.validate();
    const auto notice = request.period.start() - request.requestedOn;
    if (notice > std::chrono::days{maximumAdvanceDays} ||
        (!allowSameDay && notice == std::chrono::days{0})) {
        throw InvalidPeriod("Срок предварительного бронирования вне правил проката.");
    }
}

bool HoldPolicy::permitsPickup(const Period& period, Day day) const {
    return period.contains(day) && !expired(period, day);
}

bool HoldPolicy::expired(const Period& period, Day day) const {
    auto grace = std::chrono::days{pickupGraceDays};
    return day >= period.end() || day > period.start() + grace;
}

} // namespace lab2
