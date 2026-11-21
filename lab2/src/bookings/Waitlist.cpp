#include "lab2/bookings/Waitlist.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/WaitRequest.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidReservationState.h"
#include "lab2/errors/UnknownWaitRequest.h"

#include <utility>
#include <vector>

namespace lab2 {

Id Waitlist::enqueue(BookingRequest request) {
    request.validate();
    const auto id = nextId_;
    rows_.emplace(id, WaitRequest{id, std::move(request), false});
    ++nextId_;
    return id;
}

const WaitRequest& Waitlist::find(Id id) const {
    const auto match = rows_.find(id);
    if (match == rows_.end()) {
        throw UnknownWaitRequest("Заявка ожидания не найдена.");
    }
    return match->second;
}

void Waitlist::fulfil(Id id) {
    if (find(id).fulfilled) {
        throw InvalidReservationState("Заявка уже обработана.");
    }
    rows_.at(id).fulfilled = true;
}

std::vector<Id> Waitlist::candidates(Id asset, Day day) const {
    std::vector<Id> result;
    for (const auto& [id, row] : rows_) {
        if (row.matches(asset, day)) {
            result.push_back(id);
        }
    }
    return result;
}

} // namespace lab2
