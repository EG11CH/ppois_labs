#include "lab2/bookings/Cancellation.h"
#include "lab2/bookings/HoldPolicy.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidReservationState.h"

namespace lab2 {

void Cancellation::verify(Id id, Day bookedOn) const {
    if (id != reservationId || day < bookedOn || reason.empty()) {
        throw InvalidReservationState("Для отмены нужны правильный номер, дата и причина.");
    }
}

bool Cancellation::isNoShow(const HoldPolicy& policy, const Period& period) const {
    return policy.expired(period, day);
}

} // namespace lab2
