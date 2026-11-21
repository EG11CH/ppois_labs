#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/customers/CustomerAccount.h"

namespace lab2 {

struct Eligibility {
    int maximumBookings;
    int maximumContracts;
    bool requireReachable;
    void checkBooking(const CustomerAccount& account, const Period& period, int current) const;
    void checkIssue(const CustomerAccount& account, Day day, int current) const;
};

} // namespace lab2
