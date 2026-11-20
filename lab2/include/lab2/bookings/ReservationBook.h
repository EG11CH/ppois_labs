#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/HoldPolicy.h"
#include "lab2/bookings/Reservation.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/pricing/Quote.h"

#include <map>
#include <vector>

namespace lab2 {

class ReservationBook {
  public:
    Id store(BookingRequest request, Quote quote);
    const Reservation& find(Id id) const;
    Reservation& edit(Id id);
    bool conflicts(Id asset, const Period& period, Id except = 0) const;
    bool hasUpcoming(Id asset, Day day) const;
    int heldFor(Id customer, Id except = 0) const;
    int expire(Day day, const HoldPolicy& policy);
    std::vector<Id> ids() const;

  private:
    std::map<Id, Reservation> rows_;
    Id nextId_ = 1;
};

} // namespace lab2
