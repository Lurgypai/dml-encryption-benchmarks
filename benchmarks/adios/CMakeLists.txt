cmake_minimum_required(VERSION 3.2)

project(adios-benchmark)

find_package(adios2 CONFIG REQUIRED
  PATHS "../../dependencies/adios-ins/lib/cmake/adios2"
  NO_DEFAULT_PATH)

add_executable(adios-benchmark "adios-benchmark.cpp")

target_link_libraries(adios-benchmark adios2::adios2)
