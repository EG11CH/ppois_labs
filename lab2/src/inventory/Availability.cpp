#include "lab2/inventory/Availability.h"
#include "lab2/errors/AssetUnavailable.h"
#include "lab2/errors/BookingConflict.h"

namespace lab2 {

bool Availability::canReserve() const {
    return physicallyReady && !booked;
}

void Availability::requireAvailable() const {
    if (!physicallyReady) {
        throw AssetUnavailable("Предмет пока недоступен.");
    }
    if (booked) {
        throw BookingConflict("Этот период пересекается с действующей бронью.");
    }
}

} // namespace lab2
