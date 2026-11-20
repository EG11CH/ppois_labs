#pragma once

#include "lab2/pricing.h"

#include <optional>

namespace lab2 {

struct BookingRequest {
    Id customerId;
    Id assetId;
    Day requestedOn;
    Period period;
    std::string voucher;
    void validate() const;
    bool concerns(Id asset, const Period& interval) const;
};

struct HoldPolicy {
    int maximumAdvanceDays;
    int pickupGraceDays;
    bool allowSameDay;
    void checkRequest(const BookingRequest& request) const;
    bool permitsPickup(const Period& period, Day day) const;
    bool expired(const Period& period, Day day) const;
};

struct Cancellation {
    Id reservationId;
    Day day;
    std::string reason;
    void verify(Id id, Day bookedOn) const;
    bool isNoShow(const HoldPolicy& policy, const Period& period) const;
};

struct ReservationChange {
    Id reservationId;
    Day day;
    Period replacement;
    std::string reason;
    void verify(Id id, Day bookedOn) const;
    BookingRequest applyTo(const BookingRequest& original) const;
};

enum class ReservationState { held, cancelled, issued };

class Reservation {
  public:
    Reservation(Id id, BookingRequest request, Quote quote);
    Id id() const;
    ReservationState state() const;
    const BookingRequest& request() const;
    const Quote& quote() const;
    const std::optional<Cancellation>& cancellation() const;
    void requireHeld() const;
    bool blocks(Id asset, const Period& period) const;
    void cancel(Cancellation cancellation);
    void reschedule(BookingRequest request, Quote quote);
    void redeem();

  private:
    Id id_;
    BookingRequest request_;
    Quote quote_;
    ReservationState state_ = ReservationState::held;
    std::optional<Cancellation> cancellation_;
};

class ReservationBook {
  public:
    Id store(BookingRequest request, Quote quote);
    const Reservation& find(Id id) const;
    Reservation& edit(Id id);
    bool conflicts(Id asset, const Period& period, Id except = 0) const;
    bool hasUpcoming(Id asset, Day day) const;
    int heldFor(Id customer, Id except = 0) const;
    int expire(Day day, const HoldPolicy& policy);
    std::vector<Id> ids() const;

  private:
    std::map<Id, Reservation> rows_;
    Id nextId_ = 1;
};

struct WaitRequest {
    Id id;
    BookingRequest request;
    bool fulfilled;
    bool matches(Id asset, Day day) const;
    BookingRequest reconsider(Day day) const;
};

class Waitlist {
  public:
    Id enqueue(BookingRequest request);
    const WaitRequest& find(Id id) const;
    void fulfil(Id id);
    std::vector<Id> candidates(Id asset, Day day) const;

  private:
    std::map<Id, WaitRequest> rows_;
    Id nextId_ = 1;
};

} // namespace lab2
