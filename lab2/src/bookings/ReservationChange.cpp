#include "lab2/bookings/ReservationChange.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidReservationState.h"

namespace lab2 {

void ReservationChange::verify(Id id, Day bookedOn) const {
    if (id != reservationId || day < bookedOn || reason.empty()) {
        throw InvalidReservationState("Для переноса нужны правильный номер, дата и причина.");
    }
}

BookingRequest ReservationChange::applyTo(const BookingRequest& original) const {
    return {original.customerId, original.assetId, day, replacement, original.voucher};
}

} // namespace lab2
