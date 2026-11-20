#include "console.h"
#include "lab2/desk/demo.h"

#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include <string_view>

namespace {

std::string runCommands(std::string_view commands, lab2::RentalDesk& desk) {
    std::istringstream input{std::string{commands}};
    std::ostringstream output;
    REQUIRE(lab2::runConsole(input, output, desk) == 0);
    return output.str();
}

} // namespace

TEST_CASE("Консоль отклоняет ошибочный ввод и применяет только полные команды", "[console]") {
    auto desk = lab2::demoDesk();
    const auto output = runCommands(R"(
help
unknown
available
available 101x 2026-02-05 2026-02-08
catalog extra
exit extra
asset-add 103 1 SN-1 "Лыжи" Demo "Спорт" 2 40000 1000 6000 2026-02-01
asset-add 103 1 SN-1 "Лыжи" Demo "Спорт" 0 40000 1000 6000 2026-02-01
asset-retire 103 2026-02-02
catalog
exit
customer-add 3 "Пропущенная команда" +375291234567 DEMO-3 2026-02-01 2030-01-01
)",
                                    desk);

    CHECK(output.find("help | exit") != std::string::npos);
    CHECK(output.find("Неизвестная команда") != std::string::npos);
    CHECK(output.find("Не хватает аргументов") != std::string::npos);
    CHECK(output.find("Ожидается целое число") != std::string::npos);
    CHECK(output.find("Лишние аргументы") != std::string::npos);
    CHECK(output.find("Для флага укажи 0 или 1.") != std::string::npos);
    CHECK(output.find("Клиент #3 зарегистрирован.") == std::string::npos);
    CHECK(desk.inventory().find(103).state() == lab2::AssetState::retired);
    CHECK(runCommands("", desk) == "Прокат инвентаря. Список команд: help\n");
}

TEST_CASE("Консоль передаёт команды очереди, проката и обслуживания в модель", "[console]") {
    auto desk = lab2::demoDesk();
    const auto output = runCommands(R"(
customer-add 3 "Новый клиент" +375291234567 DEMO-3 2026-02-01 2030-01-01
customer-suspend 3
customer-restore 3
available 101 2026-02-05 2026-02-08
quote 1 101 2026-02-01 2026-02-05 2026-02-08 -
reserve 1 101 2026-02-01 2026-02-05 2026-02-08 -
available 101 2026-02-05 2026-02-08
wait 3 101 2026-02-01 2026-02-05 2026-02-08 -
waiting 101 2026-02-02
move 1 2026-02-02 2026-02-08 2026-02-10 "Другие даты"
promote 1 2026-02-02
cancel 2 2026-02-02 "По просьбе"
expire 2026-02-02
status
issue 1 2026-02-08 15000 1 1 1 0 "Нет инструкции"
issue 1 2026-02-08 15000 1 1 1 1 -
catalog
available 101 2026-02-10 2026-02-12
status
extend 1 2026-02-09 2026-02-11 "Ещё день"
return 1 1 2026-02-12 1 0 1 10000 "Трещина корпуса"
catalog
service-start 101 2026-02-12 "Замена корпуса"
service-finish 101 1 2026-02-13 2400
status
)",
                                    desk);

    CHECK(output.find("Клиент #3 разблокирован.") != std::string::npos);
    CHECK(output.find("Аренда 71.25 BYN") != std::string::npos);
    CHECK(output.find("Пересечение с бронью.") != std::string::npos);
    CHECK(output.find("Заявки ожидания: #1") != std::string::npos);
    CHECK(output.find("Бронь #2 отменена.") != std::string::npos);
    CHECK(output.find("Выдача требует осмотра без дефектов.") != std::string::npos);
    CHECK(output.find("Договор #1 продлён до 2026-02-11.") != std::string::npos);
    CHECK(output.find("итого 183.25 BYN, доплата 33.25 BYN") != std::string::npos);
    CHECK(output.find("закрыт 2026-02-13") != std::string::npos);
    CHECK_FALSE(desk.contracts().find(1).isActive());
    CHECK(desk.inventory().find(101).canIssue());
}
