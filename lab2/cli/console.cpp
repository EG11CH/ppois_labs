#include "console.h"
#include "lab2/desk.h"

#include <algorithm>
#include <charconv>
#include <functional>
#include <iomanip>
#include <sstream>
#include <utility>

namespace lab2 {
namespace {

struct Command {
    std::string_view name;
    std::string_view arguments;
    std::function<void(std::istream&, std::ostream&, RentalDesk&)> execute;
};

std::string word(std::istream& input) {
    std::string value;
    if (!(input >> std::quoted(value))) {
        throw std::invalid_argument("Не хватает аргументов или кавычка не закрыта.");
    }
    return value;
}

std::string optionalText(std::istream& input) {
    auto value = word(input);
    return value == "-" ? "" : value;
}

template <typename Integer = Id> Integer number(std::istream& input) {
    const auto token = word(input);
    Integer value{};
    const auto* last = token.data() + token.size();
    const auto [end, error] = std::from_chars(token.data(), last, value);
    if (error != std::errc{} || end != last) {
        throw std::invalid_argument("Ожидается целое число: " + token);
    }
    return value;
}

bool flag(std::istream& input) {
    const auto value = number(input);
    if (value != 0 && value != 1) {
        throw std::invalid_argument("Для флага укажи 0 или 1.");
    }
    return value == 1;
}

Day date(std::istream& input) {
    return parseDay(word(input));
}

Period period(std::istream& input) {
    return {date(input), date(input)};
}

void endOfCommand(std::istream& input) {
    input >> std::ws;
    if (!input.eof()) {
        throw std::invalid_argument("Лишние аргументы. См. help.");
    }
}

BookingRequest booking(std::istream& input) {
    return {number(input), number(input), date(input), period(input), optionalText(input)};
}

InspectionChecklist checklist(std::istream& input) {
    return {flag(input), flag(input), flag(input)};
}

std::string_view stateName(AssetState state) {
    switch (state) {
    case AssetState::ready: return "готов";
    case AssetState::rented: return "выдан";
    case AssetState::service: return "обслуживание";
    case AssetState::retired: return "списан";
    }
    throw std::logic_error("Неизвестное состояние предмета.");
}

std::string_view stateName(ReservationState state) {
    switch (state) {
    case ReservationState::held: return "действует";
    case ReservationState::cancelled: return "отменена";
    case ReservationState::issued: return "выдано";
    }
    throw std::logic_error("Неизвестное состояние брони.");
}

void showQuote(std::ostream& output, const Quote& quote) {
    output << "Аренда " << formatMoney(quote.rentalDue()) << ", скидка "
           << formatMoney(quote.discount) << ", залог " << formatMoney(quote.dueAtPickup()) << '\n';
}

void catalog(std::istream& input, std::ostream& output, const RentalDesk& desk) {
    endOfCommand(input);
    for (const auto id : desk.inventory().assetIds()) {
        const auto& asset = desk.inventory().find(id);
        output << '#' << id << ' ' << asset.model().label() << " | пункт " << asset.branch()
               << " | " << stateName(asset.state()) << " | " << asset.serial() << '\n';
    }
}

void available(std::istream& input, std::ostream& output, const RentalDesk& desk) {
    const auto id = number(input);
    const auto interval = period(input);
    endOfCommand(input);
    const auto result = desk.availability(id, interval);
    output << (result.canReserve() ? "Можно забронировать."
               : result.booked     ? "Пересечение с бронью."
                                   : "Предмет недоступен.")
           << '\n';
}

void quote(std::istream& input, std::ostream& output, const RentalDesk& desk) {
    const auto request = booking(input);
    endOfCommand(input);
    showQuote(output, desk.preview(request));
}

void reserve(std::istream& input, std::ostream& output, RentalDesk& desk) {
    auto request = booking(input);
    endOfCommand(input);
    const auto id = desk.reserve(std::move(request));
    output << "Бронь #" << id << '\n';
    showQuote(output, desk.reservations().find(id).quote());
}

void move(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const ReservationChange change{number(input), date(input), period(input), word(input)};
    endOfCommand(input);
    desk.reschedule(change);
    output << "Бронь #" << change.reservationId << " перенесена.\n";
}

void cancel(std::istream& input, std::ostream& output, RentalDesk& desk) {
    Cancellation cancellation{number(input), date(input), word(input)};
    endOfCommand(input);
    const auto id = cancellation.reservationId;
    desk.cancel(std::move(cancellation));
    output << "Бронь #" << id << " отменена.\n";
}

void expire(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto day = date(input);
    endOfCommand(input);
    output << "Снято просроченных броней: " << desk.expire(day) << '\n';
}

void wait(std::istream& input, std::ostream& output, RentalDesk& desk) {
    auto request = booking(input);
    endOfCommand(input);
    const auto id = desk.joinWaitlist(std::move(request));
    output << "Заявка ожидания #" << id << '\n';
}

void waiting(std::istream& input, std::ostream& output, const RentalDesk& desk) {
    const auto asset = number(input);
    const auto day = date(input);
    endOfCommand(input);
    const auto ids = desk.waitingFor(asset, day);
    output << "Заявки ожидания:";
    for (const auto id : ids) {
        output << " #" << id;
    }
    output << '\n';
}

void promote(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto id = number(input);
    const auto day = date(input);
    endOfCommand(input);
    const auto result = desk.promote(id, day);
    output << "Создана бронь #" << result << " из заявки #" << id << ".\n";
}

CheckoutRecord readCheckout(std::istream& input, const RentalDesk& desk, Id reservation, Day day) {
    const auto briefed = flag(input);
    const auto checks = checklist(input);
    auto note = optionalText(input);
    endOfCommand(input);
    const auto assetId = desk.reservations().find(reservation).request().assetId;
    const auto branch = desk.inventory().find(assetId).branch();
    HandoverKit kit{assetId, {{"manual", "Инструкция", true}}, {}};
    if (checks.accessoriesComplete) {
        kit.present.emplace_back("manual");
    }
    return {reservation,
            day,
            {1, branch, "Оператор смены"},
            {assetId, day, checks, std::move(note)},
            std::move(kit),
            briefed};
}

void issue(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto reservation = number(input);
    const auto day = date(input);
    const auto deposit = number<Money>(input);
    const auto id = desk.issue(readCheckout(input, desk, reservation, day), deposit);
    output << "Договор #" << id << '\n' << desk.contracts().find(id).receipt().summary() << '\n';
}

void extend(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const ExtensionRequest request{number(input), date(input), date(input), word(input)};
    endOfCommand(input);
    desk.extend(request);
    output << "Договор #" << request.contractId << " продлён до " << formatDay(request.until)
           << ".\n";
}

void returnAsset(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto contract = number(input);
    const auto branch = number(input);
    const auto day = date(input);
    const auto checks = checklist(input);
    const auto estimate = number<Money>(input);
    auto note = optionalText(input);
    endOfCommand(input);
    const auto asset = desk.contracts().find(contract).booking().assetId;
    const auto& invoice =
        desk.acceptReturn({contract, branch, day, {asset, day, checks, std::move(note)}, estimate});
    output << invoice.summary() << '\n';
    const auto& incident = desk.contracts().find(contract).incident();
    if (incident) {
        output << incident->summary() << '\n';
    }
}

void serviceStart(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto asset = number(input);
    const auto day = date(input);
    auto reason = word(input);
    endOfCommand(input);
    const auto id = desk.startService(asset, day, std::move(reason));
    output << desk.serviceLog().find(id).summary() << '\n';
}

void serviceFinish(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto asset = number(input);
    const auto id = number(input);
    const auto day = date(input);
    const auto labor = number<Money>(input);
    endOfCommand(input);
    desk.finishService(asset, id, day, labor, {});
    output << desk.serviceLog().find(id).summary() << '\n';
}

void status(std::istream& input, std::ostream& output, const RentalDesk& desk) {
    endOfCommand(input);
    for (const auto id : desk.reservations().ids()) {
        const auto& row = desk.reservations().find(id);
        output << "Бронь #" << id << " " << stateName(row.state());
        if (const auto& cancellation = row.cancellation()) {
            output << " " << formatDay(cancellation->day) << ": " << cancellation->reason;
        }
        output << '\n';
    }
    for (const auto id : desk.contracts().ids()) {
        const auto& contract = desk.contracts().find(id);
        output << "Договор #" << id
               << (contract.isActive() ? " действует, до " : " закрыт, срок был до ")
               << formatDay(contract.quote().period.end()) << '\n';
    }
}

void customerAdd(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto id = number(input);
    auto name = word(input);
    auto phone = word(input);
    auto document = word(input);
    const auto joined = date(input);
    const auto expires = date(input);
    endOfCommand(input);
    desk.enroll({id,
                 {std::move(name),
                  {std::move(phone), "", false},
                  {std::move(document), "Указан при регистрации", expires},
                  joined},
                 {"Обычный клиент", 0, expires}});
    output << "Клиент #" << id << " зарегистрирован.\n";
}

void customerSuspend(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto id = number(input);
    endOfCommand(input);
    desk.suspendCustomer(id);
    output << "Клиент #" << id << " заблокирован.\n";
}

void customerRestore(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto id = number(input);
    endOfCommand(input);
    desk.restoreCustomer(id);
    output << "Клиент #" << id << " разблокирован.\n";
}

void assetAdd(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto id = number(input);
    const auto branch = number(input);
    auto serial = word(input);
    AssetModel model{
        word(input), word(input), {"CUSTOM", word(input), flag(input)}, number<Money>(input)};
    const Tariff tariff{number<Money>(input), number<Money>(input), 1};
    const auto day = date(input);
    endOfCommand(input);
    desk.addAsset({id, branch, std::move(serial), std::move(model)}, tariff, day);
    output << "Предмет #" << id << " добавлен.\n";
}

void assetRetire(std::istream& input, std::ostream& output, RentalDesk& desk) {
    const auto id = number(input);
    const auto day = date(input);
    endOfCommand(input);
    desk.retireAsset(id, day);
    output << "Предмет #" << id << " списан." << '\n';
}

const Command commands[]{
    {"catalog", "", catalog},
    {"available", "ITEM START END", available},
    {"quote", "CUSTOMER ITEM TODAY START END VOUCHER|-", quote},
    {"reserve", "CUSTOMER ITEM TODAY START END VOUCHER|-", reserve},
    {"move", "BOOKING TODAY START END REASON", move},
    {"cancel", "BOOKING TODAY REASON", cancel},
    {"expire", "TODAY", expire},
    {"wait", "CUSTOMER ITEM TODAY START END VOUCHER|-", wait},
    {"waiting", "ITEM TODAY", waiting},
    {"promote", "WAIT_ID TODAY", promote},
    {"issue", "BOOKING DAY DEPOSIT BRIEFED POWER CASE KIT NOTE|-", issue},
    {"extend", "CONTRACT TODAY NEW_END REASON", extend},
    {"return", "CONTRACT BRANCH DAY POWER CASE KIT DAMAGE NOTE|-", returnAsset},
    {"service-start", "ITEM DAY REASON", serviceStart},
    {"service-finish", "ITEM JOB DAY LABOR", serviceFinish},
    {"status", "", status},
    {"asset-retire", "ITEM DAY", assetRetire},
    {"customer-add", "ID NAME PHONE DOCUMENT JOINED EXPIRES", customerAdd},
    {"customer-suspend", "ID", customerSuspend},
    {"customer-restore", "ID", customerRestore},
    {"asset-add", "ID BRANCH SERIAL MODEL MAKER CATEGORY BRIEFING VALUE DAILY WEEKLY DAY",
     assetAdd},
};

void help(std::ostream& output) {
    output << "Даты ГГГГ-ММ-ДД, суммы в копейках, флаги 0/1. Текст с пробелами — в кавычках.\n"
           << "help | exit\n";
    for (const auto& command : commands) {
        output << command.name << ' ' << command.arguments << '\n';
    }
}

void execute(std::string_view name, std::istream& input, std::ostream& output, RentalDesk& desk) {
    if (name == "help") {
        endOfCommand(input);
        help(output);
        return;
    }
    const auto found =
        std::find_if(std::begin(commands), std::end(commands),
                     [name](const Command& command) { return command.name == name; });
    if (found == std::end(commands)) {
        throw std::invalid_argument("Неизвестная команда. Введи help.");
    }
    found->execute(input, output, desk);
}

} // namespace

int runConsole(std::istream& input, std::ostream& output, RentalDesk& desk) {
    output << "Прокат инвентаря. Список команд: help\n";
    for (std::string line; std::getline(input, line);) {
        std::istringstream arguments{line};
        std::string name;
        if (!(arguments >> name)) {
            continue;
        }
        try {
            if (name == "exit") {
                endOfCommand(arguments);
                return 0;
            }
            execute(name, arguments, output, desk);
        } catch (const std::exception& error) {
            output << "Ошибка: " << error.what() << '\n';
        }
    }
    return 0;
}

} // namespace lab2
