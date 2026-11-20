#pragma once

#include "lab2/bookings.h"
#include "lab2/customers.h"

namespace lab2 {

struct Accessory {
    std::string code;
    std::string label;
    bool required;
    bool missingFrom(const std::vector<std::string>& present) const;
    std::string description() const;
};

struct HandoverKit {
    Id assetId;
    std::vector<Accessory> expected;
    std::vector<std::string> present;
    std::vector<std::string> missing() const;
    void verify(Id asset) const;
};

struct CheckoutRecord {
    Id reservationId;
    Day day;
    Operator clerk;
    Inspection inspection;
    HandoverKit kit;
    bool briefingGiven;
    void verify(const Reservation& reservation, const Asset& asset, const HoldPolicy& policy) const;
};

struct ReturnRecord {
    Id contractId;
    Id branchId;
    Day day;
    Inspection inspection;
    Money damageEstimate;
    void verify(Id contract, const Asset& asset, const Branch& branch, Day issued) const;
    DamageAssessment assessment(const AssetModel& model) const;
};

struct ExtensionRequest {
    Id contractId;
    Day requestedOn;
    Day until;
    std::string reason;
    Period applyTo(Id contract, Day issued, const Period& current) const;
};

struct Receipt {
    Id contractId;
    std::string clerkSignature;
    Day day;
    Payment payment;
    void verifyDeposit(const Quote& quote) const;
    std::string summary() const;
};

struct Incident {
    Id contractId;
    Id assetId;
    Day day;
    std::string description;
    DamageAssessment assessment;
    bool requiresService() const;
    std::string summary() const;
};

struct Invoice {
    Id contractId;
    Day issuedOn;
    Settlement settlement;
    Payment finalPayment;
    void verify() const;
    Money netCollected(const Receipt& receipt) const;
    std::string summary() const;
};

class RentalContract {
  public:
    RentalContract(Id id, BookingRequest booking, CheckoutRecord checkout, Quote quote,
                   Receipt receipt);
    Id id() const;
    const BookingRequest& booking() const;
    Day issuedOn() const;
    const Quote& quote() const;
    const Receipt& receipt() const;
    const std::optional<Invoice>& invoice() const;
    const std::optional<Incident>& incident() const;
    bool isActive() const;
    void requireActive() const;
    void extend(Quote newQuote);
    Invoice prepareInvoice(const ReturnRecord& returned, const AssetModel& model,
                           const LateFeePolicy& policy) const;
    std::optional<Incident> inspectIncident(const ReturnRecord& returned,
                                            const AssetModel& model) const;
    void close(Invoice invoice, std::optional<Incident> incident);

  private:
    Id id_;
    BookingRequest booking_;
    CheckoutRecord checkout_;
    Quote quote_;
    Receipt receipt_;
    std::optional<Invoice> invoice_;
    std::optional<Incident> incident_;
};

class ContractBook {
  public:
    Id nextId() const;
    void store(RentalContract contract);
    const RentalContract& find(Id id) const;
    RentalContract& edit(Id id);
    int activeFor(Id customer) const;
    std::vector<Id> ids() const;

  private:
    std::map<Id, RentalContract> rows_;
    Id nextId_ = 1;
};

} // namespace lab2
