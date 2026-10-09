add_library(lab1_options INTERFACE)
target_compile_features(lab1_options INTERFACE cxx_std_17)
target_compile_options(lab1_options INTERFACE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall;-Wextra;-Wpedantic;-Wconversion;-Wshadow;-Werror>)

if(ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(FATAL_ERROR "Coverage requires GCC and gcov")
    endif()
    target_compile_options(lab1_options INTERFACE --coverage -O0 -g)
    target_link_options(lab1_options INTERFACE --coverage)
endif()
if(ENABLE_SANITIZERS)
    target_compile_options(lab1_options INTERFACE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(lab1_options INTERFACE -fsanitize=address,undefined)
endif()

add_library(lab1_domain
    src/EnglishRussianDictionary.cpp src/Set.cpp src/SetElement.cpp)
target_include_directories(lab1_domain PUBLIC include)
target_link_libraries(lab1_domain PUBLIC lab1_options)

add_library(lab1_cli cli/Console.cpp cli/DictionaryMenu.cpp cli/SetMenu.cpp)
target_include_directories(lab1_cli PUBLIC cli)
target_link_libraries(lab1_cli PUBLIC lab1_domain)

add_executable(dictionary_cli app/dictionary_main.cpp)
add_executable(set_cli app/set_main.cpp)
target_link_libraries(dictionary_cli PRIVATE lab1_cli)
target_link_libraries(set_cli PRIVATE lab1_cli)
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
    target_link_libraries(lab1_tests PRIVATE lab1_cli Catch2::Catch2WithMain)
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
