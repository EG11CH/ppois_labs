cmake_minimum_required(VERSION 3.20)
project(OopLaboratories VERSION 1.0 LANGUAGES CXX)

include(CTest)
option(ENABLE_COVERAGE "Build with GCC coverage instrumentation" OFF)
option(ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)
option(BUILD_PORTABLE_BINARIES "Link Linux release executables statically" OFF)
add_subdirectory(lab1)
