cmake_minimum_required(VERSION 3.10)
project(recycle)

# Define library
add_library(recycle INTERFACE)
target_compile_features(recycle INTERFACE cxx_std_14)
target_include_directories(recycle INTERFACE src/)
add_library(steinwurf::recycle ALIAS recycle)

# Install headers
install(
  DIRECTORY ./src/recycle
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include
  FILES_MATCHING
  PATTERN *.hpp)
