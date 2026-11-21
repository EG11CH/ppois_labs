#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidPeriod.h"
#include "lab2/errors/InvalidReservationState.h"

namespace lab2 {

void BookingRequest::validate() const {
    if (customerId <= 0 || assetId <= 0) {
        throw InvalidReservationState("Нужны положительные номера клиента и предмета.");
    }
    if (requestedOn > period.start()) {
        throw InvalidPeriod("Бронь нельзя создать задним числом.");
    }
}

bool BookingRequest::concerns(Id asset, const Period& interval) const {
    return assetId == asset && period.overlaps(interval);
}

} // namespace lab2
