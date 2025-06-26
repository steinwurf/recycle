cmake_minimum_required(VERSION 3.10)
project(recycle)


# Include common CMake settings
include("${STEINWURF_RESOLVE}/toolchains/common_settings.cmake")

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

  # Is top level project?
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})

  # Setup testing
  enable_testing()

  # Google Test dependency
  add_subdirectory("${STEINWURF_RESOLVE}/gtest" EXCLUDE_FROM_ALL)

  # Build test executable
  file(GLOB_RECURSE recycle_test_sources ./test/*.cpp)


  add_executable(recycle_test ${recycle_test_sources})
  target_link_libraries(recycle_test recycle)
  target_link_libraries(recycle_test steinwurf::gtest)

  add_test(NAME recycle_test COMMAND recycle_test)

endif()
