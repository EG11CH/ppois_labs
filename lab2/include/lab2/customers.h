#pragma once

#include "lab2/common.h"

#include <map>

namespace lab2 {

struct Contact {
    std::string phone;
    std::string email;
    bool acceptsEmail;
    bool canReceiveEmail() const;
    bool hasPhone() const;
    std::string notificationTarget() const;
};

struct IdentityDocument {
    std::string number;
    std::string issuer;
    Day expires;
    bool complete() const;
    bool validOn(Day day) const;
};

struct CustomerProfile {
    std::string name;
    Contact contact;
    IdentityDocument document;
    Day joined;
    void validate(Day asOf) const;
    bool canIdentify(Day day) const;
};

struct Membership {
    std::string name;
    int discountPercent;
    Day validUntil;
    int discountOn(Day day) const;
    void renew(Day newDeadline);
};

class CustomerAccount {
  public:
    CustomerAccount(Id id, CustomerProfile profile, Membership membership);
    Id id() const;
    const CustomerProfile& profile() const;
    const Membership& membership() const;
    void suspend();
    void restore();
    bool canRentOn(Day day) const;

  private:
    Id id_;
    CustomerProfile profile_;
    Membership membership_;
    bool suspended_ = false;
};

struct Eligibility {
    int maximumBookings;
    int maximumContracts;
    bool requireReachable;
    void checkBooking(const CustomerAccount& account, const Period& period, int current) const;
    void checkIssue(const CustomerAccount& account, Day day, int current) const;
};

class CustomerRegistry {
  public:
    explicit CustomerRegistry(Eligibility rules);
    void enroll(CustomerAccount account);
    const CustomerAccount& find(Id id) const;
    void suspend(Id id);
    void restore(Id id);
    void verifyBooking(Id id, const Period& period, int current) const;
    void verifyIssue(Id id, Day day, int current) const;

  private:
    std::map<Id, CustomerAccount> accounts_;
    Eligibility rules_;
};

struct Operator {
    Id id;
    Id branchId;
    std::string name;
    bool mayHandle(Id branch) const;
    std::string signature() const;
};

} // namespace lab2
