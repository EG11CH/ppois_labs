#include "lab2/customers.h"

#include <utility>

namespace lab2 {

bool Contact::canReceiveEmail() const {
    const auto separator = email.find('@');
    return acceptsEmail && separator != std::string::npos && separator > 0 &&
           separator + 1 < email.size() && email.find('@', separator + 1) == std::string::npos &&
           email.find_first_of(" \t\r\n") == std::string::npos;
}

bool Contact::hasPhone() const {
    constexpr std::size_t minDigits = 7;
    constexpr std::size_t maxDigits = 15;
    const auto digits = phone.starts_with('+') ? std::string_view{phone}.substr(1) : phone;
    return digits.size() >= minDigits && digits.size() <= maxDigits &&
           digits.find_first_not_of("0123456789") == std::string_view::npos;
}

std::string Contact::notificationTarget() const {
    if (canReceiveEmail()) {
        return email;
    }
    return hasPhone() ? phone : "";
}

bool IdentityDocument::complete() const {
    return !number.empty() && !issuer.empty();
}

bool IdentityDocument::validOn(Day day) const {
    return complete() && day <= expires;
}

void CustomerProfile::validate(Day asOf) const {
    if (name.find_first_not_of(" \t") == std::string::npos || !document.validOn(asOf) ||
        asOf < joined || contact.notificationTarget().empty()) {
        throw InvalidCustomer("Проверь имя, действующий документ, дату регистрации и контакты.");
    }
}

bool CustomerProfile::canIdentify(Day day) const {
    return day >= joined && document.validOn(day);
}

int Membership::discountOn(Day day) const {
    if (discountPercent < 0 || discountPercent > maxPercent) {
        throw InvalidPayment("Процент скидки вне диапазона 0–100.");
    }
    return day <= validUntil ? discountPercent : 0;
}

void Membership::renew(Day newDeadline) {
    if (newDeadline <= validUntil) {
        throw InvalidCustomer("Новый срок членства должен быть позже текущего.");
    }
    validUntil = newDeadline;
}

CustomerAccount::CustomerAccount(Id id, CustomerProfile profile, Membership membership)
    : id_{id}, profile_{std::move(profile)}, membership_{std::move(membership)} {
    if (id_ <= 0) {
        throw InvalidCustomer("Нужен положительный номер клиента.");
    }
    profile_.validate(profile_.joined);
    membership_.discountOn(profile_.joined);
}

Id CustomerAccount::id() const {
    return id_;
}

const CustomerProfile& CustomerAccount::profile() const {
    return profile_;
}

const Membership& CustomerAccount::membership() const {
    return membership_;
}

void CustomerAccount::suspend() {
    suspended_ = true;
}

void CustomerAccount::restore() {
    suspended_ = false;
}

bool CustomerAccount::canRentOn(Day day) const {
    return !suspended_ && profile_.canIdentify(day);
}

void Eligibility::checkBooking(const CustomerAccount& account, const Period& period,
                               int current) const {
    if (!account.canRentOn(period.end()) || !account.canRentOn(period.start()) ||
        current >= maximumBookings ||
        (requireReachable && account.profile().contact.notificationTarget().empty())) {
        throw InvalidCustomer("Клиент не может забронировать инвентарь на этот срок.");
    }
}

void Eligibility::checkIssue(const CustomerAccount& account, Day day, int current) const {
    if (!account.canRentOn(day) || current >= maximumContracts) {
        throw InvalidCustomer("Выдача клиенту недоступна.");
    }
}

CustomerRegistry::CustomerRegistry(Eligibility rules) : rules_{rules} {
}

void CustomerRegistry::enroll(CustomerAccount account) {
    const auto id = account.id();
    if (!accounts_.try_emplace(id, std::move(account)).second) {
        throw DuplicateId("Клиент с таким номером уже есть.");
    }
}

const CustomerAccount& CustomerRegistry::find(Id id) const {
    const auto account = accounts_.find(id);
    if (account == accounts_.end()) {
        throw UnknownCustomer("Клиент не найден.");
    }
    return account->second;
}

void CustomerRegistry::suspend(Id id) {
    find(id);
    accounts_.at(id).suspend();
}

void CustomerRegistry::restore(Id id) {
    find(id);
    accounts_.at(id).restore();
}

void CustomerRegistry::verifyBooking(Id id, const Period& period, int current) const {
    rules_.checkBooking(find(id), period, current);
}

void CustomerRegistry::verifyIssue(Id id, Day day, int current) const {
    rules_.checkIssue(find(id), day, current);
}

bool Operator::mayHandle(Id branch) const {
    return id > 0 && branchId == branch && !name.empty();
}

std::string Operator::signature() const {
    return name + " #" + std::to_string(id);
}

} // namespace lab2
