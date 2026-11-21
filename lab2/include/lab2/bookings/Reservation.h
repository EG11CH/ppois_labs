#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/Cancellation.h"
#include "lab2/bookings/ReservationState.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/pricing/Quote.h"

#include <optional>

namespace lab2 {

class Reservation {
  public:
    Reservation(Id id, BookingRequest request, Quote quote);
    Id id() const;
    ReservationState state() const;
    const BookingRequest& request() const;
    const Quote& quote() const;
    const std::optional<Cancellation>& cancellation() const;
    void requireHeld() const;
    bool blocks(Id asset, const Period& period) const;
    void cancel(Cancellation cancellation);
    void reschedule(BookingRequest request, Quote quote);
    void redeem();

  private:
    Id id_;
    BookingRequest request_;
    Quote quote_;
    ReservationState state_ = ReservationState::held;
    std::optional<Cancellation> cancellation_;
};

} // namespace lab2
