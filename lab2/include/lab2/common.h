#pragma once

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace lab2 {

using Day = std::chrono::sys_days;
using Id = int;
using Money = std::int64_t; // копейки
inline constexpr Money maxInputAmount = 100'000'000;
inline constexpr int maxRentalDays = 365;
inline constexpr int maxPercent = 100;

class InvalidPeriod : public std::runtime_error {
    using runtime_error::runtime_error;
};
class InvalidCustomer : public std::runtime_error {
    using runtime_error::runtime_error;
};
class DuplicateId : public std::runtime_error {
    using runtime_error::runtime_error;
};
class UnknownCustomer : public std::runtime_error {
    using runtime_error::runtime_error;
};
class UnknownAsset : public std::runtime_error {
    using runtime_error::runtime_error;
};
class UnknownBranch : public std::runtime_error {
    using runtime_error::runtime_error;
};
class AssetUnavailable : public std::runtime_error {
    using runtime_error::runtime_error;
};
class BookingConflict : public std::runtime_error {
    using runtime_error::runtime_error;
};
class UnknownReservation : public std::runtime_error {
    using runtime_error::runtime_error;
};
class InvalidReservationState : public std::runtime_error {
    using runtime_error::runtime_error;
};
class UnknownContract : public std::runtime_error {
    using runtime_error::runtime_error;
};
class InvalidContractState : public std::runtime_error {
    using runtime_error::runtime_error;
};
class InvalidPayment : public std::runtime_error {
    using runtime_error::runtime_error;
};
class InvalidInspection : public std::runtime_error {
    using runtime_error::runtime_error;
};
class MaintenanceError : public std::runtime_error {
    using runtime_error::runtime_error;
};
class UnknownWaitRequest : public std::runtime_error {
    using runtime_error::runtime_error;
};
class InvalidInventory : public std::runtime_error {
    using runtime_error::runtime_error;
};

class Period {
  public:
    Period(Day start, Day end);
    bool contains(Day day) const;
    bool overlaps(const Period& other) const;
    bool covers(const Period& other) const;
    int days() const;

    Day start() const;
    Day end() const;

  private:
    Day start_;
    Day end_;
};

Day parseDay(std::string_view text);
std::string formatDay(Day day);
std::string formatMoney(Money amount);
void validateInputAmount(Money amount);

} // namespace lab2
