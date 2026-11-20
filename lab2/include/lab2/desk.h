#pragma once

#include "lab2/maintenance.h"
#include "lab2/rentals.h"

namespace lab2 {

struct RentalPolicy {
    Eligibility eligibility;
    HoldPolicy holds;
    DepositPolicy deposits;
    LateFeePolicy lateFees;
    Voucher voucher;
    ServiceRule service;
    int discountCap;
    Quote price(const BookingRequest& request, const CustomerAccount& account, const Asset& asset,
                const Tariff& tariff) const;
};

struct AssetTerms {
    Tariff tariff;
    MaintenancePlan maintenance;
    Day lastMovement;
    bool readyFor(const Asset& asset, const Period& period) const;
    void verifyTimeline(Day day) const;
};

class RentalDesk {
  public:
    explicit RentalDesk(RentalPolicy policy);
    void addBranch(Branch branch);
    void enroll(CustomerAccount account);
    void addAsset(Asset asset, Tariff tariff, Day commissioned);
    void retireAsset(Id asset, Day day);
    void suspendCustomer(Id customer);
    void restoreCustomer(Id customer);
    Availability availability(Id asset, const Period& period, Id except = 0) const;
    Quote preview(const BookingRequest& request, Id except = 0) const;
    Id reserve(BookingRequest request);
    void cancel(Cancellation cancellation);
    void reschedule(const ReservationChange& change);
    int expire(Day day);
    Id joinWaitlist(BookingRequest request);
    Id promote(Id waiting, Day day);
    std::vector<Id> waitingFor(Id asset, Day day) const;
    Id issue(CheckoutRecord checkout, Money paidDeposit);
    void extend(const ExtensionRequest& request);
    const Invoice& acceptReturn(const ReturnRecord& returned);
    Id startService(Id asset, Day day, std::string reason);
    void finishService(Id asset, Id job, Day day, Money labor, std::vector<PartUsage> parts);
    const Inventory& inventory() const;
    const ReservationBook& reservations() const;
    const ContractBook& contracts() const;
    const ServiceLog& serviceLog() const;

  private:
    RentalPolicy policy_;
    CustomerRegistry customers_;
    Inventory inventory_;
    ReservationBook reservations_;
    Waitlist waiting_;
    ContractBook contracts_;
    MaintenanceDesk maintenance_;
    std::map<Id, AssetTerms> terms_;
};

RentalDesk demoDesk();

} // namespace lab2
