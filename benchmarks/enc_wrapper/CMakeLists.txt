cmake_minimum_required(VERSION 3.2)

project(enc_wrapper-benchmark)

find_package(enc_wrapper)
find_package(enc_io)

add_executable(enc_wrapper-benchmark "enc_wrapper-benchmark.cpp")

target_link_libraries(enc_wrapper-benchmark enc_wrapper::enc_io)
