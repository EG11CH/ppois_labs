#include "lab2/desk.h"

#include <catch2/catch_test_macros.hpp>
#include <limits>

using namespace lab2;

namespace {
Day date(int number) {
    return parseDay("2026-02-01") + std::chrono::days{number - 1};
}

AssetModel model() {
    return {"Дрель", "Demo", {"TOOLS", "Инструменты", true}, 50'000};
}

CustomerAccount account() {
    return {
        9,
        {"Анна", {"+375291111111", "anna@example.test", true}, {"ABC", "Пункт", date(17)}, date(1)},
        {"Постоянный", 15, date(10)}};
}
} // namespace

TEST_CASE("Периоды используют полуоткрытые интервалы и календарные даты", "[model]") {
    const Period booked{date(2), date(5)};
    CHECK(booked.days() == 3);
    CHECK(booked.contains(date(2)));
    CHECK_FALSE(booked.contains(date(5)));
    CHECK_FALSE(booked.overlaps({date(5), date(6)}));
    CHECK(booked.overlaps({date(4), date(6)}));
    CHECK(booked.covers({date(3), date(5)}));
    CHECK_FALSE(booked.covers({date(2), date(6)}));
    CHECK(formatDay(parseDay("2024-02-29")) == "2024-02-29");
    CHECK_THROWS_AS(parseDay("2026-02-29"), InvalidPeriod);
    CHECK_THROWS_AS(parseDay("2026/02/01"), InvalidPeriod);
    CHECK_THROWS_AS(parseDay("2026-0a-01"), InvalidPeriod);
    CHECK_THROWS_AS(parseDay("2026--0-01"), InvalidPeriod);
    CHECK_THROWS_AS(parseDay("1999-12-31"), InvalidPeriod);
    CHECK_THROWS_AS((Period{date(2), date(2)}), InvalidPeriod);
    CHECK_THROWS_AS((Period{date(2), date(2) + std::chrono::days{366}}), InvalidPeriod);
    CHECK(formatMoney(5) == "0.05 BYN");
    CHECK(formatMoney(-105) == "-1.05 BYN");
    CHECK(formatMoney(std::numeric_limits<Money>::min()) == "-92233720368547758.08 BYN");
    CHECK_THROWS_AS(validateInputAmount(maxInputAmount + 1), InvalidPayment);
    CHECK_THROWS_AS(validateInputAmount(-1), InvalidPayment);
}

TEST_CASE("Контакты, документы и лимиты определяют допуск клиента", "[model]") {
    CHECK((Contact{"+1234567", "a@b.test", true}.notificationTarget() == "a@b.test"));
    CHECK((Contact{"1234567", "a@b.test", false}.notificationTarget() == "1234567"));
    CHECK((Contact{"123", "a@b@c", true}.notificationTarget().empty()));
    CHECK_FALSE((Contact{"12ABCDE", "a b@c", true}.hasPhone()));
    CHECK_FALSE((IdentityDocument{"", "Issuer", date(7)}.complete()));
    CHECK_FALSE((IdentityDocument{"ABC", "Issuer", date(7)}.validOn(date(8))));
    auto member = account().membership();
    CHECK(member.discountOn(date(10)) == 15);
    CHECK(member.discountOn(date(11)) == 0);
    CHECK_THROWS_AS(member.renew(date(10)), InvalidCustomer);
    member.renew(date(12));
    CHECK(member.discountOn(date(12)) == 15);
    CHECK_THROWS_AS((Membership{"x", 101, date(7)}.discountOn(date(1))), InvalidPayment);
    CHECK_THROWS_AS((CustomerAccount{0, account().profile(), member}), InvalidCustomer);
    auto profile = account().profile();
    profile.name = " ";
    CHECK_THROWS_AS(profile.validate(date(1)), InvalidCustomer);
    CHECK_FALSE(profile.canIdentify(date(0)));

    CustomerRegistry registry{{1, 1, true}};
    registry.enroll(account());
    CHECK_THROWS_AS(registry.enroll(account()), DuplicateId);
    CHECK_THROWS_AS(registry.find(404), UnknownCustomer);
    CHECK_NOTHROW(registry.verifyBooking(9, {date(1), date(17)}, 0));
    CHECK_THROWS_AS(registry.verifyBooking(9, {date(1), date(18)}, 0), InvalidCustomer);
    CHECK_THROWS_AS(registry.verifyBooking(9, {date(1), date(2)}, 1), InvalidCustomer);
    CHECK_THROWS_AS(registry.verifyIssue(9, date(1), 1), InvalidCustomer);
    registry.suspend(9);
    CHECK_THROWS_AS(registry.verifyIssue(9, date(1), 0), InvalidCustomer);
    registry.restore(9);
    CHECK_NOTHROW(registry.verifyIssue(9, date(1), 0));
    const Operator clerk{1, 2, "Анна"};
    CHECK(clerk.mayHandle(2));
    CHECK_FALSE(clerk.mayHandle(1));
    CHECK(clerk.signature() == "Анна #1");
}

TEST_CASE("Инвентарь, осмотр и комплектность защищают переходы состояний", "[model]") {
    Inventory stock;
    CHECK_THROWS_AS(stock.addBranch({0, "Центр", "Адрес", true}), InvalidInventory);
    CHECK_THROWS_AS(stock.addAsset({10, 2, "SN", model()}), UnknownBranch);
    stock.addBranch({1, "Центр", "Адрес", true});
    CHECK_THROWS_AS(stock.addBranch({1, "Центр", "Адрес", true}), DuplicateId);
    CHECK(stock.branch(1).label() == "Центр — Адрес");
    CHECK(stock.branch(1).acceptsAt(1));
    CHECK_FALSE((Branch{1, "А", "Б", false}.acceptsAt(1)));
    CHECK_THROWS_AS((Asset{0, 1, "SN", model()}), InvalidInventory);
    CHECK_THROWS_AS((Category{"", "Инструменты", true}.validate()), InvalidInventory);
    auto invalidModel = model();
    invalidModel.replacementValue = 0;
    CHECK_THROWS_AS(invalidModel.validate(), InvalidInventory);
    CHECK(model().label() == "Demo Дрель");
    stock.addAsset({10, 1, "SN", model()});
    CHECK_THROWS_AS(stock.addAsset({10, 1, "SN2", model()}), DuplicateId);
    CHECK_THROWS_AS(stock.addAsset({11, 1, "SN", model()}), DuplicateId);
    CHECK_THROWS_AS(stock.find(404), UnknownAsset);
    CHECK(stock.readyAt(1) == std::vector<Id>{10});
    CHECK(stock.assetIds() == std::vector<Id>{10});
    auto& asset = stock.edit(10);
    CHECK_THROWS_AS(asset.receive(Condition::good), InvalidContractState);
    CHECK_THROWS_AS(asset.releaseFromService(), MaintenanceError);
    asset.issue();
    CHECK(stock.count(AssetState::rented) == 1);
    CHECK(stock.readyAt(1).empty());
    CHECK_THROWS_AS(asset.issue(), AssetUnavailable);
    CHECK_THROWS_AS(asset.sendToService(), MaintenanceError);
    CHECK_THROWS_AS(asset.retire(), InvalidInventory);
    asset.receive(Condition::damaged);
    CHECK(asset.state() == AssetState::service);
    asset.releaseFromService();
    asset.retire();
    CHECK_FALSE(asset.canIssue());
    const Inspection good{10, date(2), {true, true, true}, ""};
    const Inspection bad{10, date(2), {false, false, true}, "Корпус"};
    CHECK_NOTHROW(good.verifyBeforeIssue(10, date(2)));
    CHECK(bad.checklist.defects() == 2);
    CHECK(bad.condition() == Condition::damaged);
    CHECK_THROWS_AS(good.verifyFor(20, date(2)), InvalidInspection);
    CHECK_THROWS_AS(bad.verifyBeforeIssue(10, date(2)), InvalidInspection);
    const HandoverKit kit{10, {{"manual", "Инструкция", true}, {"box", "Коробка", false}}, {"box"}};
    CHECK(kit.missing() == std::vector<std::string>{"Инструкция (manual)"});
    CHECK_THROWS_AS(kit.verify(10), InvalidInspection);
}

TEST_CASE("Тарифы и расчёт залога проверяются по целым копейкам", "[model]") {
    const Period eightDays{date(1), date(9)};
    const Tariff tariff{2'500, 15'000, 1};
    CHECK(tariff.rentalFor(eightDays) == 17'500);
    CHECK((Tariff{2'500, 15'000, 3}.rentalFor({date(1), date(2)}) == 7'500));
    CHECK_THROWS_AS((Tariff{100, 701, 1}.validate()), InvalidPayment);
    CHECK_THROWS_AS((Tariff{100, 99, 1}.validate()), InvalidPayment);
    CHECK_THROWS_AS((Tariff{100, 700, 0}.validate()), InvalidPayment);
    const Discount discount{15, 20, 18};
    CHECK(discount.effectivePercent() == 18);
    CHECK(discount.saving(101) == 18);
    CHECK_THROWS_AS((Discount{0, -1, 100}.effectivePercent()), InvalidPayment);
    const Voucher coupon{"WEEK10", {date(1), date(10)}, 10, 7};
    CHECK(coupon.redeem("WEEK10", eightDays) == 10);
    CHECK(coupon.redeem("", eightDays) == 0);
    CHECK_THROWS_AS(coupon.redeem("OTHER", eightDays), InvalidPayment);
    CHECK_THROWS_AS(coupon.redeem("WEEK10", {date(1), date(3)}), InvalidPayment);
    const DepositPolicy deposits{30, 1'000, 12'000};
    CHECK(deposits.holdFor(model()) == 12'000);
    CHECK_THROWS_AS((DepositPolicy{30, 13'000, 12'000}.holdFor(model())), InvalidPayment);
    const LateFeePolicy late{1'200, 1, 5};
    CHECK(late.fee(date(9), date(8)) == 0);
    CHECK(late.fee(date(9), date(11)) == 1'200);
    CHECK(late.fee(date(9), date(18)) == 6'000);
    CHECK_THROWS_AS((LateFeePolicy{1'200, -1, 5}.fee(date(1), date(2))), InvalidPayment);
    const DamageAssessment damage{Condition::damaged, 60'000, 50'000};
    CHECK(damage.charge() == 50'000);
    CHECK(damage.uncoveredLoss() == 10'000);
    CHECK_THROWS_AS((DamageAssessment{Condition::good, 1, 100}.charge()), InvalidInspection);
    const Quote quote{eightDays, 17'500, 1'750, 12'000};
    const Settlement settlement{quote, 1'200, 0, 12'000};
    const Invoice invoice{1, date(11), settlement, {1, PaymentKind::balance, 4'950, date(11)}};
    const Receipt receipt{1, "Clerk", date(1), {1, PaymentKind::deposit, 12'000, date(1)}};
    CHECK(invoice.netCollected(receipt) == 16'950);
    CHECK(invoice.summary().find("49.50 BYN") != std::string::npos);
    CHECK(receipt.summary().find("Clerk") != std::string::npos);
    CHECK_THROWS_AS(
        (Payment{2, PaymentKind::balance, 10, date(1)}.verify(2, PaymentKind::refund, 10, date(1))),
        InvalidPayment);
    CHECK_THROWS_AS(
        (Payment{2, PaymentKind::refund, 10, date(1)}.verify(2, PaymentKind::refund, 10, date(2))),
        InvalidPayment);
    CHECK((Payment{2, PaymentKind::refund, 10, date(2)}.movement() == -10));
    CHECK_THROWS_AS(invoice.netCollected({2, "Clerk", date(1), receipt.payment}), InvalidPayment);
}

TEST_CASE("Правила выдачи, переноса и ожидания ограничивают операции с бронью", "[model]") {
    const Period original{date(4), date(9)};
    const BookingRequest request{9, 10, date(1), original, ""};
    const Quote quote{original, 500, 0, 300};
    const HoldPolicy policy{10, 1, false};
    CHECK_NOTHROW(policy.checkRequest(request));
    CHECK_THROWS_AS(policy.checkRequest({9, 10, date(4), original, ""}), InvalidPeriod);
    CHECK_THROWS_AS(policy.checkRequest({9, 10, date(-10), original, ""}), InvalidPeriod);
    CHECK_THROWS_AS(policy.checkRequest({9, 10, date(5), original, ""}), InvalidPeriod);
    CHECK_THROWS_AS(policy.checkRequest({0, 10, date(1), original, ""}), InvalidReservationState);
    CHECK(policy.permitsPickup(original, date(5)));
    CHECK_FALSE(policy.permitsPickup(original, date(3)));
    CHECK((Cancellation{1, date(6), "Неявка"}.isNoShow(policy, original)));
    CHECK_THROWS_AS((Cancellation{2, date(6), "Причина"}.verify(1, date(1))),
                    InvalidReservationState);
    CHECK_THROWS_AS((ReservationChange{2, date(1), original, "Причина"}.verify(1, date(1))),
                    InvalidReservationState);
    ReservationBook book;
    const auto id = book.store(request, quote);
    CHECK_THROWS_AS(book.find(404), UnknownReservation);
    CHECK(book.heldFor(9) == 1);
    CHECK(book.conflicts(10, original));
    CHECK(book.hasUpcoming(10, date(8)));
    CHECK_FALSE(book.conflicts(10, original, id));
    CHECK(book.expire(date(5), policy) == 0);
    CHECK(book.expire(date(6), policy) == 1);
    CHECK_FALSE(book.hasUpcoming(10, date(6)));
    CHECK_THROWS_AS(book.edit(id).redeem(), InvalidReservationState);
    const auto second = book.store(request, quote);
    CHECK_THROWS_AS(book.edit(second).reschedule({9, 11, date(1), original, ""}, quote),
                    InvalidReservationState);
    Waitlist waitlist;
    const auto waiting = waitlist.enqueue(request);
    CHECK(waitlist.candidates(10, date(4)) == std::vector<Id>{waiting});
    CHECK(waitlist.candidates(10, date(5)).empty());
    CHECK_THROWS_AS(waitlist.find(404), UnknownWaitRequest);
    CHECK(waitlist.find(waiting).reconsider(date(3)).requestedOn == date(3));
    waitlist.fulfil(waiting);
    CHECK_THROWS_AS(waitlist.fulfil(waiting), InvalidReservationState);
    CHECK_THROWS_AS(waitlist.find(waiting).reconsider(date(3)), InvalidReservationState);
}

TEST_CASE("Обслуживание проверяет привязку заказа, даты, смету и историю", "[model]") {
    CHECK_THROWS_AS((MaintenanceDesk{{0, 1, true}}), MaintenanceError);
    const ServiceRule rule{2, 30, true};
    CHECK(rule.due(2, date(1), date(1), Condition::good));
    CHECK(rule.due(0, date(1), date(31), Condition::good));
    CHECK(rule.due(0, date(1), date(1), Condition::damaged));
    MaintenanceDesk maintenance{rule};
    Asset item{10, 1, "SN", model()};
    auto plan = maintenance.planFor(10, date(1));
    CHECK_FALSE(plan.recordReturn(date(2), Condition::good));
    CHECK(plan.recordReturn(date(3), Condition::good));
    CHECK_THROWS_AS(plan.recordReturn(date(0), Condition::good), MaintenanceError);
    CHECK_THROWS_AS(plan.reset(date(0)), MaintenanceError);
    CHECK(plan.dueOn(date(3)));
    CHECK_THROWS_AS(maintenance.start(item, date(3), ""), MaintenanceError);
    CHECK(item.canIssue());
    const auto job = maintenance.start(item, date(3), "Два цикла");
    CHECK(maintenance.log().find(job).id() == job);
    CHECK(maintenance.log().find(job).open());
    CHECK(maintenance.log().find(job).expense() == 0);
    CHECK(maintenance.log().find(job).summary().find("открыт") != std::string::npos);
    CHECK_THROWS_AS(maintenance.log().find(999), MaintenanceError);
    CHECK_THROWS_AS(maintenance.start(item, date(3), "Повтор"), MaintenanceError);
    const Part part{"CASE-1", "Корпус", "Поставщик", 750};
    CHECK_THROWS_AS(part.priceFor(0), MaintenanceError);
    CHECK_THROWS_AS((Part{"", "Корпус", "Поставщик", 1}.validate()), MaintenanceError);
    CHECK_THROWS_AS((PartUsage{part, 1, ""}.cost()), MaintenanceError);
    const PartUsage usage{part, 2, "Трещины"};
    CHECK(usage.summary().find("15.00 BYN") != std::string::npos);
    const auto original = plan.lastService;
    CHECK_THROWS_AS(maintenance.finish(item, plan, job, date(2), {500, {usage}, 2'000}),
                    MaintenanceError);
    CHECK_THROWS_AS(maintenance.finish(item, plan, job, date(4), {500, {usage}, 1'999}),
                    MaintenanceError);
    CHECK_THROWS_AS((RepairEstimate{maxInputAmount, {usage}, maxInputAmount}.total()),
                    MaintenanceError);
    CHECK(plan.lastService == original);
    CHECK(maintenance.log().find(job).open());
    maintenance.finish(item, plan, job, date(4), {500, {usage}, 2'000});
    CHECK_FALSE(maintenance.log().find(job).open());
    CHECK(maintenance.log().find(job).summary().find("закрыт") != std::string::npos);
    CHECK(maintenance.log().totalFor(10) == 2'000);
    CHECK(maintenance.log().totalFor(11) == 0);
    CHECK(item.canIssue());
    CHECK(plan.rentalCycles == 0);
    CHECK_THROWS_AS(maintenance.finish(item, plan, job, date(5), {0, {}, 0}), MaintenanceError);
    item.issue();
    CHECK_THROWS_AS(maintenance.start(item, date(5), "Выдано"), MaintenanceError);
}
