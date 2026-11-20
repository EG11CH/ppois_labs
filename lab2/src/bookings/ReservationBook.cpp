#include "lab2/bookings/ReservationBook.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/HoldPolicy.h"
#include "lab2/bookings/Reservation.h"
#include "lab2/bookings/ReservationState.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/UnknownReservation.h"
#include "lab2/pricing/Quote.h"

#include <utility>
#include <vector>

namespace lab2 {

Id ReservationBook::store(BookingRequest request, Quote quote) {
    const auto id = nextId_;
    rows_.try_emplace(id, id, std::move(request), quote);
    ++nextId_;
    return id;
}

const Reservation& ReservationBook::find(Id id) const {
    const auto match = rows_.find(id);
    if (match == rows_.end()) {
        throw UnknownReservation("Бронь не найдена.");
    }
    return match->second;
}

Reservation& ReservationBook::edit(Id id) {
    find(id);
    return rows_.at(id);
}

bool ReservationBook::conflicts(Id asset, const Period& period, Id except) const {
    for (const auto& [id, row] : rows_) {
        if (id != except && row.blocks(asset, period)) {
            return true;
        }
    }
    return false;
}

bool ReservationBook::hasUpcoming(Id asset, Day day) const {
    for (const auto& [id, row] : rows_) {
        if (row.state() == ReservationState::held && row.request().assetId == asset &&
            row.request().period.end() > day) {
            return true;
        }
    }
    return false;
}

int ReservationBook::heldFor(Id customer, Id except) const {
    int count = 0;
    for (const auto& [id, row] : rows_) {
        count += id != except && row.request().customerId == customer &&
                 row.state() == ReservationState::held;
    }
    return count;
}

int ReservationBook::expire(Day day, const HoldPolicy& policy) {
    int expired = 0;
    for (auto& [id, row] : rows_) {
        if (row.state() != ReservationState::held || !policy.expired(row.request().period, day)) {
            continue;
        }
        row.cancel({id, day, "Неявка за инвентарём"});
        ++expired;
    }
    return expired;
}

std::vector<Id> ReservationBook::ids() const {
    std::vector<Id> result;
    for (const auto& [id, row] : rows_) {
        result.push_back(id);
    }
    return result;
}

} // namespace lab2
