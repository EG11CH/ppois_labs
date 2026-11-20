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
    src/customers.cpp
    src/inventory.cpp
    src/pricing.cpp
    src/bookings.cpp
)
target_include_directories(lab2_domain PUBLIC include)
