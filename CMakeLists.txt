cmake_minimum_required(VERSION 3.10)

# Top-level check
if(NOT DEFINED RECYCLE_IS_TOPLEVEL)
  set(RECYCLE_IS_TOPLEVEL OFF)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(RECYCLE_IS_TOPLEVEL ON)
  endif()
endif()

project(
  recycle
  VERSION 6.0.0
  DESCRIPTION "Simple resource pool for recycling resources in C++"
  LANGUAGES CXX)

option(RECYCLE_ENABLE_TESTING "" ${RECYCLE_IS_TOPLEVEL})

# Define library
add_library(recycle INTERFACE)
target_compile_features(recycle INTERFACE cxx_std_14)
target_include_directories(
  recycle INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
                    $<INSTALL_INTERFACE:include>)
add_library(steinwurf::recycle ALIAS recycle)

# Add testing
if(RECYCLE_ENABLE_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()

include(GNUInstallDirs)
set(RECYCLE_CONFIG_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/recycle"
    CACHE PATH "recycle config directory")

# Install headers
install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/src/recycle
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  FILES_MATCHING
  PATTERN *.hpp)

# Install targets
install(TARGETS recycle EXPORT recycle-targets)
install(
  EXPORT recycle-targets
  DESTINATION ${RECYCLE_CONFIG_DIR}
  NAMESPACE steinwurf::)

# Install package config
include(CMakePackageConfigHelpers)

configure_package_config_file(
  cmake/recycle-config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/recycle-config.cmake"
  INSTALL_DESTINATION ${RECYCLE_CONFIG_DIR}
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/recycle-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/recycle-config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/recycle-config-version.cmake"
        DESTINATION ${RECYCLE_CONFIG_DIR})
