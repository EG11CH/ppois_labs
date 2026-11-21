#include "lab2/customers/CustomerAccount.h"
#include "lab2/common.h"
#include "lab2/customers/CustomerProfile.h"
#include "lab2/customers/Membership.h"
#include "lab2/errors/InvalidCustomer.h"

#include <utility>

namespace lab2 {

CustomerAccount::CustomerAccount(Id id, CustomerProfile profile, Membership membership)
    : id_{id}, profile_{std::move(profile)}, membership_{std::move(membership)} {
    if (id_ <= 0) {
        throw InvalidCustomer("Нужен положительный номер клиента.");
    }
    profile_.validate(profile_.joined);
    membership_.discountOn(profile_.joined);
}

Id CustomerAccount::id() const {
    return id_;
}

const CustomerProfile& CustomerAccount::profile() const {
    return profile_;
}

const Membership& CustomerAccount::membership() const {
    return membership_;
}

void CustomerAccount::suspend() {
    suspended_ = true;
}

void CustomerAccount::restore() {
    suspended_ = false;
}

bool CustomerAccount::canRentOn(Day day) const {
    return !suspended_ && profile_.canIdentify(day);
}

} // namespace lab2
