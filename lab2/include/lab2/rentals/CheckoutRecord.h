#pragma once

#include "lab2/bookings/HoldPolicy.h"
#include "lab2/bookings/Reservation.h"
#include "lab2/common.h"
#include "lab2/customers/Operator.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/Inspection.h"
#include "lab2/rentals/HandoverKit.h"

namespace lab2 {

struct CheckoutRecord {
    Id reservationId;
    Day day;
    Operator clerk;
    Inspection inspection;
    HandoverKit kit;
    bool briefingGiven;
    void verify(const Reservation& reservation, const Asset& asset, const HoldPolicy& policy) const;
};

} // namespace lab2
