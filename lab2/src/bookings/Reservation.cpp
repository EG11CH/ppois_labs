#include "lab2/bookings/Reservation.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/Cancellation.h"
#include "lab2/bookings/ReservationState.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/errors/InvalidReservationState.h"
#include "lab2/pricing/Quote.h"

#include <optional>
#include <utility>

namespace lab2 {

Reservation::Reservation(Id id, BookingRequest request, Quote quote)
    : id_{id}, request_{std::move(request)}, quote_{quote} {
}

Id Reservation::id() const {
    return id_;
}

ReservationState Reservation::state() const {
    return state_;
}

const BookingRequest& Reservation::request() const {
    return request_;
}

const Quote& Reservation::quote() const {
    return quote_;
}

const std::optional<Cancellation>& Reservation::cancellation() const {
    return cancellation_;
}

void Reservation::requireHeld() const {
    if (state_ != ReservationState::held) {
        throw InvalidReservationState("Операция доступна только для действующей брони.");
    }
}

bool Reservation::blocks(Id asset, const Period& period) const {
    return state_ == ReservationState::held && request_.concerns(asset, period);
}

void Reservation::cancel(Cancellation cancellation) {
    requireHeld();
    cancellation.verify(id_, request_.requestedOn);
    cancellation_.emplace(std::move(cancellation));
    state_ = ReservationState::cancelled;
}

void Reservation::reschedule(BookingRequest request, Quote quote) {
    requireHeld();
    if (request.assetId != request_.assetId || request.customerId != request_.customerId) {
        throw InvalidReservationState("Нельзя сменить предмет или клиента при переносе брони.");
    }
    request_ = std::move(request);
    quote_ = quote;
}

void Reservation::redeem() {
    requireHeld();
    state_ = ReservationState::issued;
}

} // namespace lab2
