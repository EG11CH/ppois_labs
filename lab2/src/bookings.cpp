#include "lab2/bookings.h"

#include <utility>

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

void Cancellation::verify(Id id, Day bookedOn) const {
    if (id != reservationId || day < bookedOn || reason.empty()) {
        throw InvalidReservationState("Для отмены нужны правильный номер, дата и причина.");
    }
}

bool Cancellation::isNoShow(const HoldPolicy& policy, const Period& period) const {
    return policy.expired(period, day);
}

void ReservationChange::verify(Id id, Day bookedOn) const {
    if (id != reservationId || day < bookedOn || reason.empty()) {
        throw InvalidReservationState("Для переноса нужны правильный номер, дата и причина.");
    }
}

BookingRequest ReservationChange::applyTo(const BookingRequest& original) const {
    return {original.customerId, original.assetId, day, replacement, original.voucher};
}

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
