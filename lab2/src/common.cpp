#include "lab2/common.h"

#include <charconv>
#include <iomanip>
#include <sstream>

namespace lab2 {

namespace {
constexpr auto firstDay = Day{std::chrono::year{2000} / 1 / 1};
constexpr auto lastDay = Day{std::chrono::year{2099} / 12 / 31};

int number(std::string_view text) {
    int result{};
    const auto* end = text.data() + text.size();
    auto [position, error] = std::from_chars(text.data(), end, result);
    if (error != std::errc{} || position != end || result < 0 || text.front() < '0') {
        throw InvalidPeriod("Ожидается дата ГГГГ-ММ-ДД.");
    }
    return result;
}
} // namespace

Period::Period(Day first, Day last) : start_{first}, end_{last} {
    if (first < firstDay || last > lastDay || first >= last ||
        last - first > std::chrono::days{maxRentalDays}) {
        throw InvalidPeriod("Нужен период от 1 до 365 дней в пределах 2000–2099 годов.");
    }
}

bool Period::contains(Day day) const {
    return start_ <= day && day < end_;
}

bool Period::overlaps(const Period& other) const {
    return start_ < other.end_ && other.start_ < end_;
}

bool Period::covers(const Period& other) const {
    return start_ <= other.start_ && other.end_ <= end_;
}

int Period::days() const {
    return static_cast<int>((end_ - start_).count());
}

Day Period::start() const {
    return start_;
}

Day Period::end() const {
    return end_;
}

Day parseDay(std::string_view text) {
    if (text.size() != 10 || text[4] != '-' || text[7] != '-') {
        throw InvalidPeriod("Ожидается дата ГГГГ-ММ-ДД.");
    }
    const auto year = std::chrono::year{number(text.substr(0, 4))};
    const auto month = std::chrono::month{static_cast<unsigned>(number(text.substr(5, 2)))};
    const auto day = std::chrono::day{static_cast<unsigned>(number(text.substr(8, 2)))};
    const auto calendarDate = year / month / day;
    if (!calendarDate.ok() || Day{calendarDate} < firstDay || Day{calendarDate} > lastDay) {
        throw InvalidPeriod("Укажи существующую дату в пределах 2000–2099 годов.");
    }
    return Day{calendarDate};
}

std::string formatDay(Day day) {
    const std::chrono::year_month_day date{day};
    std::ostringstream output;
    output << static_cast<int>(date.year()) << '-' << std::setfill('0') << std::setw(2)
           << static_cast<unsigned>(date.month()) << '-' << std::setw(2)
           << static_cast<unsigned>(date.day());
    return output.str();
}

std::string formatMoney(Money amount) {
    constexpr Money centsPerRuble = 100;
    std::ostringstream output;
    if (amount < 0) {
        output << '-';
    }
    const auto rubles = amount / centsPerRuble;
    const auto cents = amount % centsPerRuble;
    output << (rubles < 0 ? -rubles : rubles) << '.' << std::setfill('0') << std::setw(2)
           << (cents < 0 ? -cents : cents) << " BYN";
    return output.str();
}

void validateInputAmount(Money amount) {
    if (amount < 0 || amount > maxInputAmount) {
        throw InvalidPayment("Сумма должна быть от 0 до 100 000 000 копеек.");
    }
}

} // namespace lab2
