cmake_minimum_required (VERSION 3.10)
project(recycle)

file(GLOB recycle_headers ./src/recycle/*.hpp)

add_library(recycle INTERFACE)
add_library(steinwurf::recycle ALIAS recycle)

target_compile_features(recycle INTERFACE cxx_std_14)

target_include_directories(recycle INTERFACE src/)

install(FILES ${recycle_headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/recycle)
