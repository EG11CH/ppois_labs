#include "lab2/rentals/Invoice.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidPayment.h"
#include "lab2/pricing/PaymentKind.h"
#include "lab2/rentals/Receipt.h"

#include <string>

namespace lab2 {

void Invoice::verify() const {
    const auto kind = settlement.refundDue() > 0 ? PaymentKind::refund : PaymentKind::balance;
    finalPayment.verify(contractId, kind, settlement.refundDue() + settlement.balanceDue(),
                        issuedOn);
}

Money Invoice::netCollected(const Receipt& receipt) const {
    if (receipt.contractId != contractId || receipt.payment.amount != settlement.heldDeposit) {
        throw InvalidPayment("Чек залога не соответствует счёту.");
    }
    receipt.verifyDeposit(settlement.quote);
    verify();
    return receipt.payment.movement() + finalPayment.movement();
}

std::string Invoice::summary() const {
    return "Счёт по договору #" + std::to_string(contractId) + ": аренда " +
           formatMoney(settlement.quote.rentalDue()) + ", просрочка " +
           formatMoney(settlement.lateFee) + ", ущерб " + formatMoney(settlement.damageFee) +
           ", итого " + formatMoney(settlement.totalCharges()) + ", доплата " +
           formatMoney(settlement.balanceDue()) + ", возврат залога " +
           formatMoney(settlement.refundDue());
}

} // namespace lab2
