# Лабораторная работа №2 — прокат инвентаря

C++20 и CMake. Консольный пункт проката ведёт каталог, брони, договоры,
выдачу, возврат, очередь ожидания и обслуживание. В демонстрационном
сеансе есть клиенты №1 и №2, дрель №101 и палатка №102 в пункте №1.
Данные хранятся в памяти и заново загружаются при каждом запуске. Платежи
учебные. Даты операций задаёт оператор, поэтому примеры воспроизводимы.

## Запуск на Ubuntu 24.04

Из корня репозитория:

```bash
sudo apt-get update
sudo apt-get install -y g++ make cmake catch2 gcovr python3
cmake -S lab2 -B build-lab2 -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-lab2 --parallel 2
ctest --test-dir build-lab2 --output-on-failure
python3 lab2/tools/check_coverage.py build-lab2
./build-lab2/rental_cli
```

Отчёт покрытия — `build-lab2/coverage-lab2/index.html`. Из [Releases](https://github.com/EG11CH/ppois_labs/releases)
можно скачать `lab2-linux-x86_64.tar.gz`, распаковать командой
`tar -xzf lab2-linux-x86_64.tar.gz` и запустить `./rental_cli` без сборки.

## Сценарий проката

В консоли вводите по одной команде на строку. `help` выводит аргументы,
`exit` завершает сеанс. Суммы указываются в целых копейках BYN, даты —
в формате `ГГГГ-ММ-ДД`, флаги — `0` или `1`. Фраза с пробелами берётся
в кавычки. Дефис обозначает пустое примечание или отсутствие промокода.

```text
catalog
quote 1 101 2026-02-01 2026-02-05 2026-02-08 -
reserve 1 101 2026-02-01 2026-02-05 2026-02-08 -
issue 1 2026-02-05 15000 1 1 1 1 -
extend 1 2026-02-06 2026-02-09 "Ещё один день"
return 1 1 2026-02-10 1 0 1 10000 "Трещина корпуса"
service-start 101 2026-02-10 "Замена корпуса"
service-finish 101 1 2026-02-11 2400
status
```

- У `issue` после залога идут инструктаж, питание, корпус и комплектность.
  У `return` после даты идут питание, корпус, комплектность и оценка ущерба.
- Бронь занимает интервал `[начало, конец)`. Дата конца — штатная дата
  возврата. Физически выданный предмет нельзя бронировать повторно до его
  фактического возврата. Уже оформленные последующие брони продолжают действовать.
- Выдача принимает залог 150.00 BYN. После продления аренда стоит 95.00 BYN,
  просрочка в один день добавляет 12.00 BYN, ущерб — 100.00 BYN. Итог
  207.00 BYN, доплата составляет 57.00 BYN. После ремонта дрель снова доступна.
- Команды `move`, `cancel`, `expire` изменяют и освобождают бронь.
  `wait`, `waiting` и `promote` управляют очередью ожидания. На
  `customer-add`, `asset-add`, `asset-retire` доступны справочники.
- Демопромокод `WEEK10` действует на сроки от семи дней в пределах
  2026 года. Скидки не складываются. При досрочном возврате сохраняется
  арендная плата за согласованный срок.

## Сборка на GitHub

[Workflow Lab2](../.github/workflows/lab2.yml) проверяет форматирование
ClangFormat, Cppcheck, тесты и покрытие строго выше 90%. Из проверенной
сборки он собирает и повторно проверяет статический Linux x86-64 архив.
Теги `lab2-v*` публикуют его в Releases. Ветка для лабораторной —
`feature/lab2-rental`. Код попадает в `main` через pull request после
проверки преподавателем.

## Объектная модель

Порог проверяется по исходникам в `include/lab2`. Данные классов
инкапсулированы там, где переходы меняют состояние. Небольшие записи
передают реквизиты между операциями. `RentalDesk` координирует сценарии,
а команды CLI преобразуют строки в аргументы методов.

| Показатель | Требуется | Реализовано |
|---|---:|---:|
| Классы предметной области (без исключений и CLI) | 50 | 55 |
| Нестатические поля | 150 | 202 |
| Содержательные методы | 100 | 137 |
| Направленные ассоциации | 30 | 112 |
| Собственные классы исключений | 12 | 17 |

При подсчёте методов исключены конструкторы, простые чтения,
форматирование текста и прямое делегирование. Ассоциация учитывается один раз на пару
направленных классов по типу поля или параметра метода. Связи через
числовые идентификаторы и возвращаемые значения в число не входят.

### Период

[Объявления](include/lab2/common.h) · [Реализация](src/common.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **Period**. Полуоткрытый интервал календарных дат, ограниченный 365 днями. | `start_`, `end_` | `Period`, `contains`, `overlaps`, `covers`, `days`, `start`, `end` | — |

### Клиенты и сотрудники

[Объявления](include/lab2/customers.h) · [Реализация](src/customers.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **Contact**. Выбирает адрес для уведомления с учётом согласия на почту и проверяет телефон. | `phone`, `email`, `acceptsEmail` | `canReceiveEmail`, `hasPhone`, `notificationTarget` | — |
| **IdentityDocument**. Проверяет наличие реквизитов и срок действия на дату операции. | `number`, `issuer`, `expires` | `complete`, `validOn` | — |
| **CustomerProfile**. Проверяет анкету и возможность идентифицировать клиента. | `name`, `contact`, `document`, `joined` | `validate`, `canIdentify` | `Contact`, `IdentityDocument` |
| **Membership**. Даёт льготу до даты окончания и позволяет продлить членство. | `name`, `discountPercent`, `validUntil` | `discountOn`, `renew` | — |
| **CustomerAccount**. Хранит анкету, членство и блокировку учётной записи. | `id_`, `profile_`, `membership_`, `suspended_` | `CustomerAccount`, `id`, `profile`, `membership`, `suspend`, `restore`, `canRentOn` | `CustomerProfile`, `Membership` |
| **Eligibility**. Проверяет допуск к бронированию и выдаче с отдельными лимитами. | `maximumBookings`, `maximumContracts`, `requireReachable` | `checkBooking`, `checkIssue` | `CustomerAccount`, `Period` |
| **CustomerRegistry**. Регистрирует и ищет клиентов, применяет правила допуска. | `accounts_`, `rules_` | `CustomerRegistry`, `enroll`, `find`, `suspend`, `restore`, `verifyBooking`, `verifyIssue` | `CustomerAccount`, `Eligibility`, `Period` |
| **Operator**. Проверяет принадлежность к пункту и формирует подпись оператора. | `id`, `branchId`, `name` | `mayHandle`, `signature` | — |

### Инвентарь и осмотр

[Объявления](include/lab2/inventory.h) · [Реализация](src/inventory.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **Branch**. Пункт проката с адресом и правилом приёма возврата. | `id`, `name`, `address`, `acceptsReturns` | `acceptsAt`, `label` | — |
| **Category**. Категория инвентаря с признаком обязательного инструктажа. | `code`, `name`, `requiresBriefing` | `validate`, `permitsIssue` | — |
| **AssetModel**. Модель предмета с категорией и стоимостью замены. | `name`, `manufacturer`, `category`, `replacementValue` | `validate`, `label` | `Category` |
| **Asset**. Экземпляр с серийным номером, пунктом и контролируемыми переходами состояний. | `id_`, `branch_`, `serial_`, `model_`, `state_` | `Asset`, `id`, `branch`, `serial`, `model`, `state`, `canIssue`, `issue`, `receive`, `sendToService`, `releaseFromService`, `retire` | `AssetModel` |
| **InspectionChecklist**. Три результата осмотра и подсчёт дефектов. | `powersOn`, `casingIntact`, `accessoriesComplete` | `passed`, `defects` | — |
| **Inspection**. Привязывает осмотр к экземпляру и дате, проверяет заметку о дефектах. | `assetId`, `day`, `checklist`, `note` | `condition`, `verifyFor`, `verifyBeforeIssue` | `InspectionChecklist` |
| **Inventory**. Ведёт справочники пунктов и экземпляров, проверяет уникальность номеров. | `branches_`, `assets_` | `addBranch`, `addAsset`, `branch`, `find`, `edit`, `readyAt`, `count`, `assetIds` | `Asset`, `Branch` |
| **Availability**. Совмещает физическую готовность, период и конфликт брони. | `assetId`, `period`, `physicallyReady`, `booked` | `canReserve`, `requireAvailable` | `Period` |

### Цена и платежи

[Объявления](include/lab2/pricing.h) · [Реализация](src/pricing.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **Tariff**. Вычисляет аренду с минимальным сроком, недельной и суточной ставками. | `dailyRate`, `weeklyRate`, `minimumDays` | `validate`, `rentalFor` | `Period` |
| **Discount**. Выбирает наибольшую допустимую скидку и рассчитывает её в копейках. | `membershipPercent`, `voucherPercent`, `capPercent` | `effectivePercent`, `saving` | — |
| **Voucher**. Проверяет код, период действия и минимальный срок акции. | `code`, `validity`, `percent`, `minimumDays` | `applies`, `redeem` | `Period` |
| **DepositPolicy**. Считает процентный залог с учётом лимитов и стоимости замены. | `replacementPercent`, `minimum`, `maximum` | `holdFor` | `AssetModel` |
| **LateFeePolicy**. Считает оплачиваемые дни просрочки с льготой и пределом. | `perDay`, `graceDays`, `maximumDays` | `chargedDays`, `fee` | — |
| **DamageAssessment**. Проверяет оценку, ограничивает взыскание и вычисляет непокрытый ущерб. | `condition`, `estimatedRepair`, `replacementValue` | `validate`, `charge`, `uncoveredLoss` | — |
| **Quote**. Фиксирует период, арендную плату, скидку и залог. | `period`, `baseRent`, `discount`, `deposit` | `rentalDue`, `dueAtPickup` | `Period` |
| **Payment**. Проверяет реквизиты движения денег и задаёт его знак. | `contractId`, `kind`, `amount`, `day` | `matches`, `verify`, `movement` | — |
| **Settlement**. Сводит аренду, ущерб, просрочку и залог в доплату или возврат. | `quote`, `lateFee`, `damageFee`, `heldDeposit` | `totalCharges`, `balanceDue`, `refundDue` | `Quote` |

### Бронирование и ожидание

[Объявления](include/lab2/bookings.h) · [Реализация](src/bookings.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **BookingRequest**. Связывает клиента, предмет, дату запроса, период и возможный промокод. | `customerId`, `assetId`, `requestedOn`, `period`, `voucher` | `validate`, `concerns` | `Period` |
| **HoldPolicy**. Проверяет срок предварительной записи и допустимые даты получения. | `maximumAdvanceDays`, `pickupGraceDays`, `allowSameDay` | `checkRequest`, `permitsPickup`, `expired` | `BookingRequest`, `Period` |
| **Cancellation**. Подтверждает реквизиты отмены и проверяет, была ли неявка. | `reservationId`, `day`, `reason` | `verify`, `isNoShow` | `HoldPolicy`, `Period` |
| **ReservationChange**. Проверяет перенос и создаёт запрос с новым периодом. | `reservationId`, `day`, `replacement`, `reason` | `verify`, `applyTo` | `BookingRequest`, `Period` |
| **Reservation**. Хранит заявку, снимок цены, статус и сведения об отмене. | `id_`, `request_`, `quote_`, `state_`, `cancellation_` | `Reservation`, `id`, `state`, `request`, `quote`, `cancellation`, `requireHeld`, `blocks`, `cancel`, `reschedule`, `redeem` | `BookingRequest`, `Cancellation`, `Period`, `Quote` |
| **ReservationBook**. Хранит брони, ищет пересечения, считает лимиты и снимает просроченные брони. | `rows_`, `nextId_` | `store`, `find`, `edit`, `conflicts`, `hasUpcoming`, `heldFor`, `expire`, `ids` | `BookingRequest`, `HoldPolicy`, `Period`, `Quote`, `Reservation` |
| **WaitRequest**. Проверяет актуальность заявки и готовит повторную попытку бронирования. | `id`, `request`, `fulfilled` | `matches`, `reconsider` | `BookingRequest` |
| **Waitlist**. Ставит заявки в очередь по идентификатору и фиксирует исполнение. | `rows_`, `nextId_` | `enqueue`, `find`, `fulfil`, `candidates` | `BookingRequest`, `WaitRequest` |

### Договор и возврат

[Объявления](include/lab2/rentals.h) · [Реализация](src/rentals.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **Accessory**. Проверяет наличие конкретной обязательной принадлежности. | `code`, `label`, `required` | `missingFrom`, `description` | — |
| **HandoverKit**. Описывает ожидаемый и фактический комплект и перечисляет недостающее. | `assetId`, `expected`, `present` | `missing`, `verify` | `Accessory` |
| **CheckoutRecord**. Собирает доказательства готовности к выдаче и проверяет их вместе. | `reservationId`, `day`, `clerk`, `inspection`, `kit`, `briefingGiven` | `verify` | `Asset`, `HandoverKit`, `HoldPolicy`, `Inspection`, `Operator`, `Reservation` |
| **ReturnRecord**. Проверяет договор, пункт, дату и осмотр, формирует оценку ущерба. | `contractId`, `branchId`, `day`, `inspection`, `damageEstimate` | `verify`, `assessment` | `Asset`, `AssetModel`, `Branch`, `Inspection` |
| **ExtensionRequest**. Проверяет срок и причину продления, рассчитывает новый интервал. | `contractId`, `requestedOn`, `until`, `reason` | `applyTo` | `Period` |
| **Receipt**. Подтверждает приём залога по договору с подписью и датой. | `contractId`, `clerkSignature`, `day`, `payment` | `verifyDeposit`, `summary` | `Payment`, `Quote` |
| **Incident**. Описывает зарегистрированное повреждение и необходимость обслуживания. | `contractId`, `assetId`, `day`, `description`, `assessment` | `requiresService`, `summary` | `DamageAssessment` |
| **Invoice**. Проверяет итоговый платёж и сверяет чистый приход с чеком. | `contractId`, `issuedOn`, `settlement`, `finalPayment` | `verify`, `netCollected`, `summary` | `Payment`, `Receipt`, `Settlement` |
| **RentalContract**. Сохраняет сведения о выдаче, цену, чек, итоговый счёт и акт повреждения. | `id_`, `booking_`, `checkout_`, `quote_`, `receipt_`, `invoice_`, `incident_` | `RentalContract`, `id`, `booking`, `issuedOn`, `quote`, `receipt`, `invoice`, `incident`, `isActive`, `requireActive`, `extend`, `prepareInvoice`, `inspectIncident`, `close` | `AssetModel`, `BookingRequest`, `CheckoutRecord`, `Incident`, `Invoice`, `LateFeePolicy`, `Quote`, `Receipt`, `ReturnRecord` |
| **ContractBook**. Хранит договоры, выдаёт идентификатор и считает действующие договоры клиента. | `rows_`, `nextId_` | `nextId`, `store`, `find`, `edit`, `activeFor`, `ids` | `RentalContract` |

### Обслуживание

[Объявления](include/lab2/maintenance.h) · [Реализация](src/maintenance.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **ServiceRule**. Определяет необходимость обслуживания по сроку, циклам и повреждению. | `maximumCycles`, `maximumDays`, `inspectAfterDamage` | `due`, `validate` | — |
| **MaintenancePlan**. Хранит дату обслуживания и число прокатов после него. | `assetId`, `rule`, `lastService`, `rentalCycles` | `dueOn`, `recordReturn`, `reset` | `ServiceRule` |
| **Part**. Хранит реквизиты и цену детали, рассчитывает стоимость количества. | `sku`, `name`, `supplier`, `unitPrice` | `validate`, `priceFor` | — |
| **PartUsage**. Связывает деталь, количество и причину замены. | `part`, `quantity`, `reason` | `cost`, `summary` | `Part` |
| **RepairEstimate**. Складывает работу и детали, сравнивает с бюджетом. | `labor`, `parts`, `limit` | `total`, `requireWithinBudget` | `PartUsage` |
| **MaintenanceJob**. Отслеживает открытие, причину, закрытие и фактическую смету заказа. | `id_`, `assetId_`, `opened_`, `reason_`, `closed_`, `estimate_` | `MaintenanceJob`, `id`, `assetId`, `open`, `finish`, `expense`, `summary` | `RepairEstimate` |
| **ServiceLog**. Хранит заказы, проверяет единственность активного заказа и считает расходы. | `jobs_`, `nextId_` | `openFor`, `find`, `close`, `activeFor`, `totalFor` | `MaintenanceJob`, `RepairEstimate` |
| **MaintenanceDesk**. Выдаёт план, согласует предмет, заказ и дату завершения ремонта. | `rule_`, `log_` | `MaintenanceDesk`, `planFor`, `start`, `finish`, `log` | `Asset`, `MaintenancePlan`, `RepairEstimate`, `ServiceLog`, `ServiceRule` |

### Координация операций

[Объявления](include/lab2/desk.h) · [Реализация](src/desk.cpp)

| Класс и роль | Поля | Методы | Ассоциации |
|---|---|---|---|
| **RentalPolicy**. Группирует правила пункта и рассчитывает цену для заданного клиента и предмета. | `eligibility`, `holds`, `deposits`, `lateFees`, `voucher`, `service`, `discountCap` | `price` | `Asset`, `BookingRequest`, `CustomerAccount`, `DepositPolicy`, `Eligibility`, `HoldPolicy`, `LateFeePolicy`, `ServiceRule`, `Tariff`, `Voucher` |
| **AssetTerms**. Связывает тариф, план обслуживания и дату последнего движения. | `tariff`, `maintenance`, `lastMovement` | `readyFor`, `verifyTimeline` | `Asset`, `MaintenancePlan`, `Period`, `Tariff` |
| **RentalDesk**. Оркестрирует сценарии поверх реестров, очереди, правил и мастерской. | `policy_`, `customers_`, `inventory_`, `reservations_`, `waiting_`, `contracts_`, `maintenance_`, `terms_` | `RentalDesk`, `addBranch`, `enroll`, `addAsset`, `retireAsset`, `suspendCustomer`, `restoreCustomer`, `availability`, `preview`, `reserve`, `cancel`, `reschedule`, `expire`, `joinWaitlist`, `promote`, `waitingFor`, `issue`, `extend`, `acceptReturn`, `startService`, `finishService`, `inventory`, `reservations`, `contracts`, `serviceLog` | `Asset`, `AssetTerms`, `BookingRequest`, `Branch`, `Cancellation`, `CheckoutRecord`, `ContractBook`, `CustomerAccount`, `CustomerRegistry`, `ExtensionRequest`, `Inventory`, `MaintenanceDesk`, `PartUsage`, `Period`, `RentalPolicy`, `ReservationBook`, `ReservationChange`, `ReturnRecord`, `Tariff`, `Waitlist` |

## Исключения

Все наследуются от `std::runtime_error` и объявлены в [common.h](include/lab2/common.h). Консоль выводит сообщение и принимает следующую строку.

| Исключение | Причина |
|---|---|
| `InvalidPeriod` | Некорректная дата, длительность или последовательность дат. |
| `InvalidCustomer` | Некорректная анкета, просроченные документы, блокировка или превышение лимита. |
| `DuplicateId` | Повтор идентификатора или серийного номера. |
| `UnknownCustomer` | Клиент отсутствует в реестре. |
| `UnknownAsset` | Предмет отсутствует в каталоге. |
| `UnknownBranch` | Пункт отсутствует в справочнике. |
| `AssetUnavailable` | Физическое состояние или обслуживание препятствуют операции. |
| `BookingConflict` | Интервал пересекается с бронью или будущая бронь препятствует списанию. |
| `UnknownReservation` | Бронь отсутствует в журнале. |
| `InvalidReservationState` | Статус или реквизиты брони либо ожидания запрещают операцию. |
| `UnknownContract` | Договор отсутствует в журнале. |
| `InvalidContractState` | Статус договора, дата или пункт запрещают операцию. |
| `InvalidPayment` | Некорректная цена, процент, промокод, сумма либо реквизиты платежа. |
| `InvalidInspection` | Расхождение в осмотре, комплекте, инструктаже или оценке повреждения. |
| `MaintenanceError` | Неправильный заказ, план, дата обслуживания или смета. |
| `UnknownWaitRequest` | Заявка на ожидание отсутствует в очереди. |
| `InvalidInventory` | Некорректные данные инвентаря либо запрещённое списание. |
