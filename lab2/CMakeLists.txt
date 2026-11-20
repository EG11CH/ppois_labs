cmake_minimum_required(VERSION 3.20)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(Lab2Rental LANGUAGES CXX)
    include(CTest)
    option(ENABLE_COVERAGE "Build with GCC coverage instrumentation" OFF)
    option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
    option(BUILD_PORTABLE_BINARIES "Link Linux release executables statically" OFF)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror)
endif()
if(ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "Coverage requires GCC and gcov")
    endif()
    add_compile_options(--coverage -O0 -g)
    add_link_options(--coverage)
endif()
if(ENABLE_SANITIZERS)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(lab2_domain
    src/common.cpp
    src/common/Period.cpp
    src/customers/Contact.cpp
    src/customers/IdentityDocument.cpp
    src/customers/CustomerProfile.cpp
    src/customers/Membership.cpp
    src/customers/CustomerAccount.cpp
    src/customers/Eligibility.cpp
    src/customers/CustomerRegistry.cpp
    src/customers/Operator.cpp
    src/inventory/Branch.cpp
    src/inventory/Category.cpp
    src/inventory/AssetModel.cpp
    src/inventory/Asset.cpp
    src/inventory/InspectionChecklist.cpp
    src/inventory/Inspection.cpp
    src/inventory/Inventory.cpp
    src/inventory/Availability.cpp
    src/pricing/Tariff.cpp
    src/pricing/Discount.cpp
    src/pricing/Voucher.cpp
    src/pricing/DepositPolicy.cpp
    src/pricing/LateFeePolicy.cpp
    src/pricing/DamageAssessment.cpp
    src/pricing/Quote.cpp
    src/pricing/Payment.cpp
    src/pricing/Settlement.cpp
    src/bookings/BookingRequest.cpp
    src/bookings/HoldPolicy.cpp
    src/bookings/Cancellation.cpp
    src/bookings/ReservationChange.cpp
    src/bookings/Reservation.cpp
    src/bookings/ReservationBook.cpp
    src/bookings/WaitRequest.cpp
    src/bookings/Waitlist.cpp
    src/rentals/Accessory.cpp
    src/rentals/HandoverKit.cpp
    src/rentals/CheckoutRecord.cpp
    src/rentals/ReturnRecord.cpp
    src/rentals/ExtensionRequest.cpp
    src/rentals/Receipt.cpp
    src/rentals/Incident.cpp
    src/rentals/Invoice.cpp
    src/rentals/RentalContract.cpp
    src/rentals/ContractBook.cpp
    src/maintenance/ServiceRule.cpp
    src/maintenance/MaintenancePlan.cpp
    src/maintenance/Part.cpp
    src/maintenance/PartUsage.cpp
    src/maintenance/RepairEstimate.cpp
    src/maintenance/MaintenanceJob.cpp
    src/maintenance/ServiceLog.cpp
    src/maintenance/MaintenanceDesk.cpp
    src/desk/RentalPolicy.cpp
    src/desk/AssetTerms.cpp
    src/desk/RentalDesk.cpp
    src/desk/demo.cpp
)
target_include_directories(lab2_domain PUBLIC include)

add_executable(rental_cli cli/console.cpp cli/main.cpp)
target_link_libraries(rental_cli PRIVATE lab2_domain)
if(BUILD_PORTABLE_BINARIES)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "Portable static executables require Linux and GCC")
    endif()
    target_link_options(rental_cli PRIVATE -static)
endif()

if(BUILD_TESTING)
    find_package(Catch2 REQUIRED)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_executable(lab2_tests tests/model_tests.cpp tests/desk_tests.cpp)
    target_link_libraries(lab2_tests PRIVATE lab2_domain Catch2::Catch2WithMain)
    list(APPEND CMAKE_MODULE_PATH "${Catch2_DIR}")
    include(Catch)
    catch_discover_tests(lab2_tests TEST_PREFIX "lab2: ")
    add_test(NAME lab2_cli
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_test.py"
                "$<TARGET_FILE:rental_cli>")
endif()
