set(CMAKE_CXX_STANDARD 17)
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

add_library(lab1
    src/EnglishRussianDictionary.cpp src/Set.cpp src/SetElement.cpp
    cli/Console.cpp cli/DictionaryMenu.cpp cli/SetMenu.cpp)
target_include_directories(lab1 PUBLIC include cli)

add_executable(dictionary_cli cli/dictionary_main.cpp)
add_executable(set_cli cli/set_main.cpp)
target_link_libraries(dictionary_cli PRIVATE lab1)
target_link_libraries(set_cli PRIVATE lab1)
if(BUILD_PORTABLE_BINARIES)
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "Portable static executables require Linux and GCC")
    endif()
    target_link_options(dictionary_cli PRIVATE -static)
    target_link_options(set_cli PRIVATE -static)
endif()

if(BUILD_TESTING)
    find_package(Catch2 REQUIRED)
    add_executable(lab1_tests
        tests/DictionaryTests.cpp tests/SetTests.cpp tests/CliTests.cpp)
    target_link_libraries(lab1_tests PRIVATE lab1 Catch2::Catch2WithMain)
    list(APPEND CMAKE_MODULE_PATH "${Catch2_DIR}")
    include(Catch)
    catch_discover_tests(lab1_tests)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME cli_executables
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/smoke_cli.py"
        "$<TARGET_FILE:dictionary_cli>" "$<TARGET_FILE:set_cli>")
endif()

find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    configure_file(Doxyfile.in Doxyfile @ONLY)
    add_custom_target(docs
        COMMAND Doxygen::doxygen "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
