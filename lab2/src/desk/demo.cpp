#include "lab2/desk/demo.h"
#include "lab2/common.h"
#include "lab2/desk/RentalDesk.h"
#include "lab2/desk/RentalPolicy.h"

namespace lab2 {

RentalDesk demoDesk() {
    const auto opening = parseDay("2026-01-01");
    const auto expires = parseDay("2030-01-01");
    const RentalPolicy policy{{3, 2, true},
                              {90, 1, true},
                              {30, 2'000, 30'000},
                              {1'200, 0, 14},
                              {"WEEK10", {opening, parseDay("2026-12-31")}, 10, 7},
                              {10, 365, true},
                              25};
    RentalDesk desk{policy};
    desk.addBranch({1, "Центральный пункт", "ул. Парковая, 1", true});
    desk.enroll({1,
                 {"Анна",
                  {"+375291111111", "anna@example.test", true},
                  {"DEMO-1", "Учебный документ", expires},
                  opening},
                 {"Постоянный клиент", 5, expires}});
    desk.enroll(
        {2,
         {"Борис", {"+375292222222", "", false}, {"DEMO-2", "Учебный документ", expires}, opening},
         {"Обычный клиент", 0, expires}});
    desk.addAsset(
        {101, 1, "DRILL-101", {"Дрель", "DemoTool", {"TOOLS", "Инструменты", true}, 50'000}},
        {2'500, 15'000, 1}, opening);
    desk.addAsset({102, 1, "TENT-102", {"Палатка", "DemoCamp", {"OUT", "Туризм", false}, 60'000}},
                  {2'000, 12'000, 1}, opening);
    return desk;
}

} // namespace lab2
