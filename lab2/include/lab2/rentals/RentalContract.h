#pragma once

#include "lab2/bookings/BookingRequest.h"
#include "lab2/common.h"
#include "lab2/inventory/AssetModel.h"
#include "lab2/pricing/LateFeePolicy.h"
#include "lab2/pricing/Quote.h"
#include "lab2/rentals/CheckoutRecord.h"
#include "lab2/rentals/Incident.h"
#include "lab2/rentals/Invoice.h"
#include "lab2/rentals/Receipt.h"
#include "lab2/rentals/ReturnRecord.h"

#include <optional>

namespace lab2 {

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

} // namespace lab2
