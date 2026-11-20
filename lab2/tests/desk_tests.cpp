#include "lab2/bookings/BookingRequest.h"
#include "lab2/bookings/ReservationState.h"
#include "lab2/common.h"
#include "lab2/desk/demo.h"
#include "lab2/errors/AssetUnavailable.h"
#include "lab2/errors/BookingConflict.h"
#include "lab2/errors/DuplicateId.h"
#include "lab2/errors/InvalidContractState.h"
#include "lab2/errors/InvalidCustomer.h"
#include "lab2/errors/InvalidInspection.h"
#include "lab2/errors/InvalidInventory.h"
#include "lab2/errors/InvalidPayment.h"
#include "lab2/errors/InvalidPeriod.h"
#include "lab2/errors/InvalidReservationState.h"
#include "lab2/errors/MaintenanceError.h"
#include "lab2/errors/UnknownBranch.h"
#include "lab2/errors/UnknownContract.h"
#include "lab2/inventory/Asset.h"
#include "lab2/inventory/AssetState.h"
#include "lab2/pricing/Tariff.h"
#include "lab2/rentals/CheckoutRecord.h"
#include "lab2/rentals/ReturnRecord.h"

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <string>
#include <vector>

using namespace lab2;

namespace {
Day date(int number) {
    return parseDay("2026-02-01") + std::chrono::days{number - 1};
}

BookingRequest request(Id customer = 1, Id asset = 101, int start = 5, int end = 8) {
    return {customer, asset, date(1), {date(start), date(end)}, ""};
}

CheckoutRecord checkout(Id reservation, Id asset = 101, Day day = date(5)) {
    return {reservation,
            day,
            {1, 1, "Оператор"},
            {asset, day, {true, true, true}, ""},
            {asset, {{"manual", "Инструкция", true}}, {"manual"}},
            true};
}

ReturnRecord returned(Id contract, Id asset = 101, Day day = date(8)) {
    return {contract, 1, day, {asset, day, {true, true, true}, ""}, 0};
}
} // namespace

TEST_CASE("Полный цикл проката с продлением и возвратом остатка залога", "[desk]") {
    auto desk = demoDesk();
    const auto quote = desk.preview(request());
    CHECK(quote.rentalDue() == 7'125);
    const auto reservation = desk.reserve(request());
    CHECK_THROWS_AS(desk.reserve(request(2)), BookingConflict);
    CHECK(desk.availability(101, {date(8), date(10)}).canReserve());
    CHECK_THROWS_AS(desk.issue(checkout(reservation), 14'999), InvalidPayment);
    CHECK(desk.reservations().find(reservation).state() == ReservationState::held);
    CHECK(desk.inventory().find(101).canIssue());
    CHECK(desk.contracts().ids().empty());
    const auto contract = desk.issue(checkout(reservation), 15'000);
    CHECK(contract == 1);
    CHECK(desk.reservations().find(reservation).state() == ReservationState::issued);
    CHECK(desk.inventory().find(101).state() == AssetState::rented);
    CHECK_THROWS_AS(desk.reserve(request(2, 101, 10, 13)), AssetUnavailable);
    desk.extend({contract, date(6), date(9), "Нужен ещё день"});
    REQUIRE(desk.contracts().find(contract).quote().period.end() == date(9));
    CHECK_THROWS_AS(desk.extend({contract, date(5), date(10), "Задним числом"}), InvalidPeriod);
    CHECK_THROWS_AS(desk.acceptReturn(returned(contract, 101, date(5))), InvalidPeriod);
    CHECK(desk.contracts().find(contract).isActive());
    CHECK_THROWS_AS(desk.acceptReturn(returned(404)), UnknownContract);
    CHECK_THROWS_AS(desk.acceptReturn(returned(contract, 101, date(4))), InvalidContractState);
    const auto& bill = desk.acceptReturn(returned(contract, 101, date(9)));
    CHECK(bill.settlement.totalCharges() == 9'500);
    CHECK(bill.settlement.refundDue() == 5'500);
    CHECK(bill.netCollected(desk.contracts().find(contract).receipt()) == 9'500);
    CHECK(desk.contracts().find(contract).invoice().has_value());
    CHECK_FALSE(desk.contracts().find(contract).incident());
    CHECK_THROWS_AS(desk.extend({contract, date(9), date(10), "Повтор"}), InvalidContractState);
    CHECK_THROWS_AS(desk.acceptReturn(returned(contract)), InvalidContractState);
    CHECK(desk.inventory().find(101).canIssue());
    CHECK_FALSE(desk.availability(101, {date(7), date(10)}).canReserve());
}

TEST_CASE("Просрочка и повреждение создают счёт и переводят предмет в ремонт", "[desk]") {
    auto desk = demoDesk();
    const auto reservation = desk.reserve(request());
    const auto contract = desk.issue(checkout(reservation), 15'000);
    auto damage = returned(contract, 101, date(10));
    damage.damageEstimate = 10'000;
    CHECK_THROWS_AS(desk.acceptReturn(damage), InvalidInspection);
    CHECK(desk.contracts().find(contract).isActive());
    CHECK(desk.inventory().find(101).state() == AssetState::rented);
    damage.inspection.checklist.casingIntact = false;
    CHECK_THROWS_AS(desk.acceptReturn(damage), InvalidInspection);
    damage.inspection.note = "Трещина корпуса";
    const auto& invoice = desk.acceptReturn(damage);
    CHECK(invoice.settlement.lateFee == 2'400);
    CHECK(invoice.settlement.damageFee == 10'000);
    CHECK(invoice.settlement.balanceDue() == 4'525);
    REQUIRE(desk.contracts().find(contract).incident());
    CHECK(desk.contracts().find(contract).incident()->requiresService());
    CHECK(desk.contracts().find(contract).incident()->summary().find("Трещина") !=
          std::string::npos);
    CHECK(desk.inventory().find(101).state() == AssetState::service);
    CHECK_THROWS_AS(desk.startService(101, date(9), "До возврата"), InvalidPeriod);
    const auto job = desk.startService(101, date(10), "Восстановление");
    CHECK_THROWS_AS(desk.finishService(102, job, date(11), 100, {}), MaintenanceError);
    CHECK_THROWS_AS(desk.finishService(101, job, date(11), 50'001, {}), MaintenanceError);
    desk.finishService(101, job, date(11), 100, {});
    CHECK(desk.serviceLog().totalFor(101) == 100);
    CHECK(desk.availability(101, {date(11), date(13)}).canReserve());
}

TEST_CASE("Нельзя выдать неосмотренный, неподготовленный или недоступный предмет", "[desk]") {
    auto desk = demoDesk();
    const auto reservation = desk.reserve(request());
    auto issue = checkout(reservation);
    issue.briefingGiven = false;
    CHECK_THROWS_AS(desk.issue(issue, 15'000), InvalidInspection);
    issue.briefingGiven = true;
    issue.kit.present.clear();
    CHECK_THROWS_AS(desk.issue(issue, 15'000), InvalidInspection);
    issue = checkout(reservation);
    issue.clerk.branchId = 2;
    CHECK_THROWS_AS(desk.issue(issue, 15'000), InvalidReservationState);
    CHECK_THROWS_AS(desk.issue(checkout(reservation, 101, date(7)), 15'000),
                    InvalidReservationState);
    desk.suspendCustomer(1);
    CHECK_THROWS_AS(desk.issue(checkout(reservation), 15'000), InvalidCustomer);
    desk.restoreCustomer(1);
    const auto job = desk.startService(101, date(3), "Подготовка");
    CHECK_THROWS_AS(desk.issue(checkout(reservation), 15'000), AssetUnavailable);
    desk.finishService(101, job, date(4), 0, {});
    CHECK_NOTHROW(desk.issue(checkout(reservation), 15'000));
    CHECK_THROWS_AS(desk.issue(checkout(reservation), 15'000), InvalidReservationState);
}

TEST_CASE("Перенос, очередь, отмена и истечение последовательно освобождают бронь", "[desk]") {
    auto desk = demoDesk();
    CHECK_THROWS_AS(desk.joinWaitlist(request()), InvalidReservationState);
    const auto reservation = desk.reserve(request());
    const auto waiting = desk.joinWaitlist(request(2));
    CHECK(desk.waitingFor(101, date(2)) == std::vector<Id>{waiting});
    CHECK_THROWS_AS(desk.promote(waiting, date(2)), BookingConflict);
    CHECK_THROWS_AS(desk.reschedule({reservation, date(7), {date(8), date(10)}, "Поздно"}),
                    InvalidReservationState);
    desk.reschedule({reservation, date(2), {date(8), date(10)}, "Другие даты"});
    CHECK(desk.reservations().find(reservation).request().period.start() == date(8));
    const auto promoted = desk.promote(waiting, date(2));
    CHECK(desk.waitingFor(101, date(2)).empty());
    CHECK_THROWS_AS(desk.promote(waiting, date(2)), InvalidReservationState);
    desk.cancel({promoted, date(3), "Отказ"});
    CHECK(desk.reservations().find(promoted).state() == ReservationState::cancelled);
    CHECK_THROWS_AS(desk.retireAsset(101, date(3)), BookingConflict);
    CHECK(desk.expire(date(9)) == 0);
    CHECK(desk.expire(date(10)) == 1);
    desk.retireAsset(101, date(10));
    CHECK(desk.inventory().find(101).state() == AssetState::retired);
}

TEST_CASE("Последующая бронь блокирует только конфликтующую часть продления", "[desk]") {
    auto desk = demoDesk();
    const auto reservation = desk.reserve(request());
    desk.reserve(request(2, 101, 9, 11));
    const auto contract = desk.issue(checkout(reservation), 15'000);
    CHECK_THROWS_AS(desk.extend({contract, date(6), date(10), "Конфликт"}), BookingConflict);
    CHECK_THROWS_AS(desk.extend({contract, date(6), date(8), "Не продлевает"}),
                    InvalidContractState);
    desk.suspendCustomer(1);
    CHECK_THROWS_AS(desk.extend({contract, date(6), date(9), "Заблокирован"}), InvalidCustomer);
    desk.restoreCustomer(1);
    desk.extend({contract, date(6), date(9), "Соседняя дата"});
    CHECK(desk.contracts().find(contract).quote().period.end() == date(9));
    CHECK(desk.contracts().activeFor(1) == 1);
    CHECK(desk.contracts().activeFor(2) == 0);
}

TEST_CASE("Списанный предмет исключается из очереди ожидания", "[desk]") {
    auto desk = demoDesk();
    const auto reservation = desk.reserve(request());
    const auto requestInQueue = request(2, 101, 7, 10);
    const auto waiting = desk.joinWaitlist(requestInQueue);
    REQUIRE(desk.waitingFor(101, date(3)) == std::vector<Id>{waiting});

    desk.cancel({reservation, date(3), "Не нужен"});
    desk.retireAsset(101, date(3));
    CHECK(desk.waitingFor(101, date(3)).empty());
    CHECK_THROWS_AS(desk.joinWaitlist(requestInQueue), AssetUnavailable);
    CHECK_THROWS_AS(desk.promote(waiting, date(3)), AssetUnavailable);
}

TEST_CASE("Повторная регистрация и ошибки справочника сохраняют исходное состояние", "[desk]") {
    auto desk = demoDesk();
    const Asset asset{103, 2, "SN", {"Пила", "Demo", {"TOOLS", "Инструменты", false}, 1000}};
    const Tariff tariff{10, 60, 1};
    CHECK_THROWS_AS(desk.addAsset(asset, tariff, date(1)), UnknownBranch);
    desk.addBranch({3, "Закрытый", "Адрес", false});
    CHECK_THROWS_AS(desk.addAsset({103, 3, "SN", asset.model()}, tariff, date(1)),
                    InvalidInventory);
    desk.addBranch({2, "Второй", "Адрес", true});
    CHECK_NOTHROW(desk.addAsset(asset, tariff, date(1)));
    CHECK_THROWS_AS(desk.addAsset(asset, tariff, date(2)), DuplicateId);
    const auto booking = desk.reserve(request(1, 103));
    auto issue = checkout(booking, 103);
    issue.clerk.branchId = 2;
    const auto contract = desk.issue(issue, 1'000);
    auto record = returned(contract, 103);
    CHECK_THROWS_AS(desk.acceptReturn(record), InvalidContractState);
    CHECK(desk.contracts().find(contract).isActive());
    record.branchId = 2;
    CHECK_NOTHROW(desk.acceptReturn(record));
    CHECK_FALSE(desk.contracts().find(contract).isActive());
}
