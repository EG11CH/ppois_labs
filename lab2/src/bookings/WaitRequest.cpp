#include "lab2/bookings/WaitRequest.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidReservationState.h"

namespace lab2 {

bool WaitRequest::matches(Id asset, Day day) const {
    return !fulfilled && request.assetId == asset && request.requestedOn <= day &&
           day <= request.period.start();
}

BookingRequest WaitRequest::reconsider(Day day) const {
    if (!matches(request.assetId, day)) {
        throw InvalidReservationState("Заявка уже обработана или её срок прошёл.");
    }
    return {request.customerId, request.assetId, day, request.period, request.voucher};
}

} // namespace lab2
