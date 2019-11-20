cmake_minimum_required (VERSION 3.10)
project(recycle)

add_library(recycle INTERFACE)
add_library(steinwurf::recycle ALIAS recycle)

target_include_directories(recycle INTERFACE src/)
