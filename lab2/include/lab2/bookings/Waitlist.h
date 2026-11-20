#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/WaitRequest.h"
#include "lab2/common.h"

#include <map>
#include <vector>

namespace lab2 {

class Waitlist {
  public:
    Id enqueue(BookingRequest request);
    const WaitRequest& find(Id id) const;
    void fulfil(Id id);
    std::vector<Id> candidates(Id asset, Day day) const;

  private:
    std::map<Id, WaitRequest> rows_;
    Id nextId_ = 1;
};

} // namespace lab2
