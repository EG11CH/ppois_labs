#include "lab2/customers/Membership.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidCustomer.h"
#include "lab2/errors/InvalidPayment.h"

namespace lab2 {

int Membership::discountOn(Day day) const {
    if (discountPercent < 0 || discountPercent > maxPercent) {
        throw InvalidPayment("Процент скидки вне диапазона 0–100.");
    }
    return day <= validUntil ? discountPercent : 0;
}

void Membership::renew(Day newDeadline) {
    if (newDeadline <= validUntil) {
        throw InvalidCustomer("Новый срок членства должен быть позже текущего.");
    }
    validUntil = newDeadline;
}

} // namespace lab2
