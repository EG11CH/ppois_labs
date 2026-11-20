#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/Cancellation.h"
#include "lab2/bookings/ReservationBook.h"
#include "lab2/bookings/ReservationChange.h"
#include "lab2/bookings/Waitlist.h"
#include "lab2/common.h"
#include "lab2/common/Period.h"
#include "lab2/customers/CustomerAccount.h"
#include "lab2/customers/CustomerRegistry.h"
#include "lab2/desk/AssetTerms.h"
#include "lab2/desk/RentalPolicy.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/Availability.h"
#include "lab2/inventory/Branch.h"
#include "lab2/inventory/Inventory.h"
#include "lab2/maintenance/MaintenanceDesk.h"
#include "lab2/maintenance/PartUsage.h"
#include "lab2/maintenance/ServiceLog.h"
#include "lab2/pricing/Quote.h"
#include "lab2/pricing/Tariff.h"
#include "lab2/rentals/CheckoutRecord.h"
#include "lab2/rentals/ContractBook.h"
#include "lab2/rentals/ExtensionRequest.h"
#include "lab2/rentals/Invoice.h"
#include "lab2/rentals/ReturnRecord.h"

#include <map>
#include <string>
#include <vector>

namespace lab2 {

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

} // namespace lab2
