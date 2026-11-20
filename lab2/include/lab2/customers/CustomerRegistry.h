#pragma once

#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/customers/CustomerAccount.h"
#include "lab2/customers/Eligibility.h"

#include <map>

namespace lab2 {

class CustomerRegistry {
  public:
    explicit CustomerRegistry(Eligibility rules);
    void enroll(CustomerAccount account);
    const CustomerAccount& find(Id id) const;
    void suspend(Id id);
    void restore(Id id);
    void verifyBooking(Id id, const Period& period, int current) const;
    void verifyIssue(Id id, Day day, int current) const;

  private:
    std::map<Id, CustomerAccount> accounts_;
    Eligibility rules_;
};

} // namespace lab2
