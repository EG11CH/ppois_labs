#pragma once

#include "lab2/common.h"
#include "lab2/customers/CustomerProfile.h"
#include "lab2/customers/Membership.h"

namespace lab2 {

class CustomerAccount {
  public:
    CustomerAccount(Id id, CustomerProfile profile, Membership membership);
    Id id() const;
    const CustomerProfile& profile() const;
    const Membership& membership() const;
    void suspend();
    void restore();
    bool canRentOn(Day day) const;

  private:
    Id id_;
    CustomerProfile profile_;
    Membership membership_;
    bool suspended_ = false;
};

} // namespace lab2
