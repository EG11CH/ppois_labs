#include "lab2/customers/CustomerRegistry.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/customers/CustomerAccount.h"
#include "lab2/customers/Eligibility.h"
#include "lab2/errors/DuplicateId.h"
#include "lab2/errors/UnknownCustomer.h"

#include <utility>

namespace lab2 {

CustomerRegistry::CustomerRegistry(Eligibility rules) : rules_{rules} {
}

void CustomerRegistry::enroll(CustomerAccount account) {
    const auto id = account.id();
    if (!accounts_.try_emplace(id, std::move(account)).second) {
        throw DuplicateId("Клиент с таким номером уже есть.");
    }
}

const CustomerAccount& CustomerRegistry::find(Id id) const {
    const auto account = accounts_.find(id);
    if (account == accounts_.end()) {
        throw UnknownCustomer("Клиент не найден.");
    }
    return account->second;
}

void CustomerRegistry::suspend(Id id) {
    find(id);
    accounts_.at(id).suspend();
}

void CustomerRegistry::restore(Id id) {
    find(id);
    accounts_.at(id).restore();
}

void CustomerRegistry::verifyBooking(Id id, const Period& period, int current) const {
    rules_.checkBooking(find(id), period, current);
}

void CustomerRegistry::verifyIssue(Id id, Day day, int current) const {
    rules_.checkIssue(find(id), day, current);
}

} // namespace lab2
