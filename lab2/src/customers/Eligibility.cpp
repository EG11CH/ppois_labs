#include "lab2/customers/Eligibility.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/customers/CustomerAccount.h"
#include "lab2/errors/InvalidCustomer.h"

namespace lab2 {

void Eligibility::checkBooking(const CustomerAccount& account, const Period& period,
                               int current) const {
    if (!account.canRentOn(period.end()) || !account.canRentOn(period.start()) ||
        current >= maximumBookings ||
        (requireReachable && account.profile().contact.notificationTarget().empty())) {
        throw InvalidCustomer("Клиент не может забронировать инвентарь на этот срок.");
    }
}

void Eligibility::checkIssue(const CustomerAccount& account, Day day, int current) const {
    if (!account.canRentOn(day) || current >= maximumContracts) {
        throw InvalidCustomer("Выдача клиенту недоступна.");
    }
}

} // namespace lab2
