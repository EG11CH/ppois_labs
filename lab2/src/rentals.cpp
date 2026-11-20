#include "lab2/rentals.h"

#include <algorithm>
#include <utility>

namespace lab2 {

bool Accessory::missingFrom(const std::vector<std::string>& present) const {
    return required && std::find(present.begin(), present.end(), code) == present.end();
}

std::string Accessory::description() const {
    return label + " (" + code + ")";
}

std::vector<std::string> HandoverKit::missing() const {
    std::vector<std::string> result;
    for (const auto& accessory : expected) {
        if (accessory.missingFrom(present)) {
            result.push_back(accessory.description());
        }
    }
    return result;
}

void HandoverKit::verify(Id asset) const {
    if (assetId != asset || !missing().empty()) {
        throw InvalidInspection("Укомплектуй предмет перед выдачей.");
    }
}

void CheckoutRecord::verify(const Reservation& reservation, const Asset& asset,
                            const HoldPolicy& policy) const {
    reservation.requireHeld();
    const auto& request = reservation.request();
    if (reservationId != reservation.id() || asset.id() != request.assetId ||
        !clerk.mayHandle(asset.branch()) || day < request.requestedOn ||
        !policy.permitsPickup(request.period, day)) {
        throw InvalidReservationState("Проверь дату, пункт и номер выдачи.");
    }
    if (!asset.canIssue()) {
        throw AssetUnavailable("Предмет сейчас нельзя выдать.");
    }
    if (!asset.model().category.permitsIssue(briefingGiven)) {
        throw InvalidInspection("Перед выдачей проведи инструктаж.");
    }
    inspection.verifyBeforeIssue(asset.id(), day);
    kit.verify(asset.id());
}

void ReturnRecord::verify(Id contract, const Asset& asset, const Branch& branch, Day issued) const {
    if (contractId != contract || branch.id != branchId || !branch.acceptsAt(asset.branch()) ||
        asset.state() != AssetState::rented || day < issued) {
        throw InvalidContractState("Проверь договор, пункт возврата и дату.");
    }
    inspection.verifyFor(asset.id(), day);
}

DamageAssessment ReturnRecord::assessment(const AssetModel& model) const {
    return {inspection.condition(), damageEstimate, model.replacementValue};
}

Period ExtensionRequest::applyTo(Id contract, Day issued, const Period& current) const {
    if (contractId != contract || requestedOn < issued || requestedOn > current.end() ||
        until <= current.end() || reason.empty()) {
        throw InvalidContractState(
            "Продление нужно оформить до срока возврата с указанием причины.");
    }
    return {current.start(), until};
}

void Receipt::verifyDeposit(const Quote& quote) const {
    payment.verify(contractId, PaymentKind::deposit, quote.dueAtPickup(), day);
}

std::string Receipt::summary() const {
    return "Залог по договору #" + std::to_string(contractId) + ": " + formatMoney(payment.amount) +
           ", " + formatDay(day) + ", " + clerkSignature;
}

bool Incident::requiresService() const {
    return assessment.condition == Condition::damaged;
}

std::string Incident::summary() const {
    return "Предмет #" + std::to_string(assetId) + ", договор #" + std::to_string(contractId) +
           ", " + formatDay(day) + ": " + description;
}

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

Id ContractBook::nextId() const {
    return nextId_;
}

void ContractBook::store(RentalContract contract) {
    const auto id = contract.id();
    if (id != nextId_ || !rows_.try_emplace(id, std::move(contract)).second) {
        throw DuplicateId("Неверный номер нового договора.");
    }
    ++nextId_;
}

const RentalContract& ContractBook::find(Id id) const {
    const auto match = rows_.find(id);
    if (match == rows_.end()) {
        throw UnknownContract("Договор не найден.");
    }
    return match->second;
}

RentalContract& ContractBook::edit(Id id) {
    find(id);
    return rows_.at(id);
}

int ContractBook::activeFor(Id customer) const {
    int count = 0;
    for (const auto& [id, contract] : rows_) {
        count += contract.isActive() && contract.booking().customerId == customer;
    }
    return count;
}

std::vector<Id> ContractBook::ids() const {
    std::vector<Id> result;
    for (const auto& [id, contract] : rows_) {
        result.push_back(id);
    }
    return result;
}

} // namespace lab2
