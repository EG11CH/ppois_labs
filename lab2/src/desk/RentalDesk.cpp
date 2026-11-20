#include "lab2/desk/RentalDesk.h"
#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/Cancellation.h"
#include "lab2/bookings/ReservationBook.h"
#include "lab2/bookings/ReservationChange.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/customers/CustomerAccount.h"
#include "lab2/desk/AssetTerms.h"
#include "lab2/desk/RentalPolicy.h"
#include "lab2/errors/AssetUnavailable.h"
#include "lab2/errors/BookingConflict.h"
#include "lab2/errors/DuplicateId.h"
#include "lab2/errors/InvalidCustomer.h"
#include "lab2/errors/InvalidReservationState.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/inventory/Availability.h"
#include "lab2/inventory/Branch.h"
#include "lab2/inventory/Inventory.h"
#include "lab2/maintenance/PartUsage.h"
#include "lab2/maintenance/RepairEstimate.h"
#include "lab2/maintenance/ServiceLog.h"
#include "lab2/pricing/PaymentKind.h"
#include "lab2/pricing/Quote.h"
#include "lab2/pricing/Tariff.h"
#include "lab2/rentals/CheckoutRecord.h"
#include "lab2/rentals/ContractBook.h"
#include "lab2/rentals/ExtensionRequest.h"
#include "lab2/rentals/Invoice.h"
#include "lab2/rentals/Receipt.h"
#include "lab2/rentals/ReturnRecord.h"

#include <string>
#include <utility>
#include <vector>

namespace lab2 {

RentalDesk::RentalDesk(RentalPolicy policy)
    : policy_{std::move(policy)}, customers_{policy_.eligibility}, maintenance_{policy_.service} {
}

void RentalDesk::addBranch(Branch branch) {
    inventory_.addBranch(std::move(branch));
}

void RentalDesk::enroll(CustomerAccount account) {
    customers_.enroll(std::move(account));
}

void RentalDesk::addAsset(Asset asset, Tariff tariff, Day commissioned) {
    tariff.validate();
    const auto id = asset.id();
    const AssetTerms terms{tariff, maintenance_.planFor(id, commissioned), commissioned};
    if (!terms_.try_emplace(id, terms).second) {
        throw DuplicateId("Предмет с таким номером уже есть.");
    }
    try {
        inventory_.addAsset(std::move(asset));
    } catch (...) {
        terms_.erase(id);
        throw;
    }
}

void RentalDesk::retireAsset(Id asset, Day day) {
    auto& item = inventory_.edit(asset);
    auto& terms = terms_.at(asset);
    terms.verifyTimeline(day);
    if (reservations_.hasUpcoming(asset, day)) {
        throw BookingConflict("Нельзя списать предмет с будущей бронью.");
    }
    item.retire();
    terms.lastOperation = day;
}

void RentalDesk::suspendCustomer(Id customer) {
    customers_.suspend(customer);
}

void RentalDesk::restoreCustomer(Id customer) {
    customers_.restore(customer);
}

Availability RentalDesk::availability(Id asset, const Period& period, Id except) const {
    const auto& item = inventory_.find(asset);
    const auto ready = terms_.at(asset).readyFor(item, period);
    return {asset, period, ready, reservations_.conflicts(asset, period, except)};
}

Quote RentalDesk::preview(const BookingRequest& request, Id except) const {
    policy_.holds.checkRequest(request);
    const auto count = reservations_.heldFor(request.customerId, except);
    customers_.verifyBooking(request.customerId, request.period, count);
    availability(request.assetId, request.period, except).requireAvailable();
    return policy_.price(request, customers_.find(request.customerId),
                         inventory_.find(request.assetId), terms_.at(request.assetId).tariff);
}

Id RentalDesk::reserve(BookingRequest request) {
    const auto quote = preview(request);
    return reservations_.store(std::move(request), quote);
}

void RentalDesk::cancel(Cancellation cancellation) {
    reservations_.edit(cancellation.reservationId).cancel(std::move(cancellation));
}

void RentalDesk::reschedule(const ReservationChange& change) {
    auto& reservation = reservations_.edit(change.reservationId);
    reservation.requireHeld();
    change.verify(reservation.id(), reservation.request().requestedOn);
    if (policy_.holds.expired(reservation.request().period, change.day)) {
        throw InvalidReservationState("Просроченную бронь нельзя перенести.");
    }
    auto request = change.applyTo(reservation.request());
    const auto quote = preview(request, reservation.id());
    reservation.reschedule(std::move(request), quote);
}

int RentalDesk::expire(Day day) {
    return reservations_.expire(day, policy_.holds);
}

Id RentalDesk::joinWaitlist(BookingRequest request) {
    policy_.holds.checkRequest(request);
    customers_.verifyBooking(request.customerId, request.period,
                             reservations_.heldFor(request.customerId));
    const auto& asset = inventory_.find(request.assetId);
    if (asset.state() == AssetState::retired) {
        throw AssetUnavailable("Списанный предмет нельзя поставить в очередь ожидания.");
    }
    const auto availabilityNow = availability(request.assetId, request.period);
    if (availabilityNow.canReserve()) {
        throw InvalidReservationState("Предмет свободен — оформи обычную бронь.");
    }
    policy_.price(request, customers_.find(request.customerId), asset,
                  terms_.at(request.assetId).tariff);
    return waiting_.enqueue(std::move(request));
}

Id RentalDesk::promote(Id waiting, Day day) {
    const auto request = waiting_.find(waiting).reconsider(day);
    const auto reservation = reserve(request);
    waiting_.fulfil(waiting);
    return reservation;
}

std::vector<Id> RentalDesk::waitingFor(Id asset, Day day) const {
    if (inventory_.find(asset).state() == AssetState::retired) {
        return {};
    }
    return waiting_.candidates(asset, day);
}

Id RentalDesk::issue(CheckoutRecord checkout, Money paidDeposit) {
    auto& reservation = reservations_.edit(checkout.reservationId);
    const auto& request = reservation.request();
    auto& asset = inventory_.edit(request.assetId);
    auto& terms = terms_.at(asset.id());
    checkout.verify(reservation, asset, policy_.holds);
    terms.verifyTimeline(checkout.day);
    if (terms.maintenance.dueOn(checkout.day)) {
        throw MaintenanceError("Перед выдачей нужно обслужить предмет.");
    }
    customers_.verifyIssue(request.customerId, checkout.day,
                           contracts_.activeFor(request.customerId));
    const auto id = contracts_.nextId();
    Receipt receipt{id,
                    checkout.clerk.signature(),
                    checkout.day,
                    {id, PaymentKind::deposit, paidDeposit, checkout.day}};
    const auto issuedOn = checkout.day;
    contracts_.store({id, request, std::move(checkout), reservation.quote(), std::move(receipt)});
    reservation.redeem();
    asset.issue();
    terms.lastOperation = issuedOn;
    return id;
}

void RentalDesk::extend(const ExtensionRequest& request) {
    auto& contract = contracts_.edit(request.contractId);
    contract.requireActive();
    const auto period =
        request.applyTo(contract.id(), contract.issuedOn(), contract.quote().period);
    const auto& booking = contract.booking();
    auto& terms = terms_.at(booking.assetId);
    terms.verifyTimeline(request.requestedOn);
    const Period added{contract.quote().period.end(), period.end()};
    if (reservations_.conflicts(booking.assetId, added)) {
        throw BookingConflict("Продление пересекается с последующей бронью.");
    }
    const auto& account = customers_.find(booking.customerId);
    if (!account.canRentOn(period.end())) {
        throw InvalidCustomer("Клиент не может продлить договор до этой даты.");
    }
    const auto& asset = inventory_.find(booking.assetId);
    const BookingRequest extended{booking.customerId, booking.assetId, booking.requestedOn, period,
                                  booking.voucher};
    contract.extend(policy_.price(extended, account, asset, terms.tariff));
    terms.lastOperation = request.requestedOn;
}

const Invoice& RentalDesk::acceptReturn(const ReturnRecord& returned) {
    auto& contract = contracts_.edit(returned.contractId);
    auto& asset = inventory_.edit(contract.booking().assetId);
    auto& terms = terms_.at(asset.id());
    returned.verify(contract.id(), asset, inventory_.branch(returned.branchId),
                    contract.issuedOn());
    terms.verifyTimeline(returned.day);
    const auto invoice = contract.prepareInvoice(returned, asset.model(), policy_.lateFees);
    auto incident = contract.inspectIncident(returned, asset.model());
    const auto condition = returned.inspection.condition();
    auto plan = terms.maintenance;
    const bool serviceDue = plan.recordReturn(returned.day, condition);
    contract.close(invoice, std::move(incident));
    asset.receive(condition);
    if (serviceDue) {
        asset.sendToService();
    }
    terms.maintenance = plan;
    terms.lastOperation = returned.day;
    return *contract.invoice();
}

Id RentalDesk::startService(Id asset, Day day, std::string reason) {
    auto& item = inventory_.edit(asset);
    auto& terms = terms_.at(asset);
    terms.verifyTimeline(day);
    const auto job = maintenance_.start(item, day, std::move(reason));
    terms.lastOperation = day;
    return job;
}

void RentalDesk::finishService(Id asset, Id job, Day day, Money labor,
                               std::vector<PartUsage> parts) {
    auto& item = inventory_.edit(asset);
    auto& terms = terms_.at(asset);
    terms.verifyTimeline(day);
    RepairEstimate estimate{labor, std::move(parts), item.model().replacementValue};
    maintenance_.finish(item, terms.maintenance, job, day, std::move(estimate));
    terms.lastOperation = day;
}

const Inventory& RentalDesk::inventory() const {
    return inventory_;
}

const ReservationBook& RentalDesk::reservations() const {
    return reservations_;
}

const ContractBook& RentalDesk::contracts() const {
    return contracts_;
}

const ServiceLog& RentalDesk::serviceLog() const {
    return maintenance_.log();
}

} // namespace lab2
