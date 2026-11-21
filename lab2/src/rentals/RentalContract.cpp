#include "lab2/rentals/RentalContract.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/errors/InvalidContractState.h"
#include "lab2/errors/InvalidPayment.h"
#include "lab2/inventory/AssetModel.h"
#include "lab2/inventory/Condition.h"
#include "lab2/pricing/LateFeePolicy.h"
#include "lab2/pricing/Payment.h"
#include "lab2/pricing/PaymentKind.h"
#include "lab2/pricing/Quote.h"
#include "lab2/pricing/Settlement.h"
#include "lab2/rentals/CheckoutRecord.h"
#include "lab2/rentals/Incident.h"
#include "lab2/rentals/Invoice.h"
#include "lab2/rentals/Receipt.h"
#include "lab2/rentals/ReturnRecord.h"

#include <optional>
#include <utility>

namespace lab2 {

RentalContract::RentalContract(Id id, BookingRequest booking, CheckoutRecord checkout, Quote quote,
                               Receipt receipt)
    : id_{id}, booking_{std::move(booking)}, checkout_{std::move(checkout)}, quote_{quote},
      receipt_{std::move(receipt)} {
    if (receipt_.contractId != id_ || receipt_.day != checkout_.day) {
        throw InvalidPayment("Чек залога не соответствует выдаче.");
    }
    receipt_.verifyDeposit(quote_);
}

Id RentalContract::id() const {
    return id_;
}

const BookingRequest& RentalContract::booking() const {
    return booking_;
}

Day RentalContract::issuedOn() const {
    return checkout_.day;
}

const Quote& RentalContract::quote() const {
    return quote_;
}

const Receipt& RentalContract::receipt() const {
    return receipt_;
}

const std::optional<Invoice>& RentalContract::invoice() const {
    return invoice_;
}

const std::optional<Incident>& RentalContract::incident() const {
    return incident_;
}

bool RentalContract::isActive() const {
    return !invoice_;
}

void RentalContract::requireActive() const {
    if (!isActive()) {
        throw InvalidContractState("Договор уже закрыт.");
    }
}

void RentalContract::extend(Quote newQuote) {
    requireActive();
    if (!newQuote.period.covers(quote_.period) ||
        newQuote.period.start() != quote_.period.start() ||
        newQuote.period.end() <= quote_.period.end() || newQuote.deposit != quote_.deposit) {
        throw InvalidContractState("При продлении меняется срок, залог сохраняется.");
    }
    quote_ = newQuote;
}

Invoice RentalContract::prepareInvoice(const ReturnRecord& returned, const AssetModel& model,
                                       const LateFeePolicy& policy) const {
    requireActive();
    const auto late = policy.fee(quote_.period.end(), returned.day);
    const Settlement settlement{quote_, late, returned.assessment(model).charge(),
                                receipt_.payment.amount};
    const auto refund = settlement.refundDue();
    const auto kind = refund > 0 ? PaymentKind::refund : PaymentKind::balance;
    const Payment payment{id_, kind, refund > 0 ? refund : settlement.balanceDue(), returned.day};
    return {id_, returned.day, settlement, payment};
}

std::optional<Incident> RentalContract::inspectIncident(const ReturnRecord& returned,
                                                        const AssetModel& model) const {
    const auto assessment = returned.assessment(model);
    if (assessment.condition == Condition::good) {
        return std::nullopt;
    }
    return Incident{id_, booking_.assetId, returned.day, returned.inspection.note, assessment};
}

void RentalContract::close(Invoice invoice, std::optional<Incident> incident) {
    requireActive();
    if (invoice.contractId != id_ ||
        (incident && (incident->contractId != id_ || incident->assetId != booking_.assetId))) {
        throw InvalidContractState("Счёт или акт относится к другому договору.");
    }
    invoice.netCollected(receipt_);
    invoice_.emplace(std::move(invoice));
    incident_ = std::move(incident);
}

} // namespace lab2
