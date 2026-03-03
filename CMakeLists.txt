cmake_minimum_required(VERSION 3.14)
project(cio VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

# Find utf8 dependency
find_library(UTF8_LIB utf8)
find_path(UTF8_INCLUDE utf8.h)

if(NOT UTF8_LIB OR NOT UTF8_INCLUDE)
  message(FATAL_ERROR "libutf8 not found. Install it first: https://github.com/wmacevoy/utf8")
endif()

# Static library
add_library(cio_static STATIC src/cio.c)
target_include_directories(cio_static PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_include_directories(cio_static PRIVATE ${UTF8_INCLUDE})
target_link_libraries(cio_static PRIVATE ${UTF8_LIB})
set_target_properties(cio_static PROPERTIES OUTPUT_NAME cio)

# Shared library
add_library(cio_shared SHARED src/cio.c)
target_include_directories(cio_shared PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_include_directories(cio_shared PRIVATE ${UTF8_INCLUDE})
target_link_libraries(cio_shared PRIVATE ${UTF8_LIB})
set_target_properties(cio_shared PROPERTIES
  OUTPUT_NAME cio
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

# Install
include(GNUInstallDirs)

install(TARGETS cio_static cio_shared
  EXPORT cioTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES include/cio.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT cioTargets
  FILE cioTargets.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cio)

# CMake package config
include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cioConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cioConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cio)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/cioConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/cioConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cioConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cio)

# pkg-config
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
set(INCLUDEDIR ${CMAKE_INSTALL_FULL_INCLUDEDIR})
configure_file(cio.pc.in cio.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cio.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Tests (optional, requires facts library)
option(CIO_BUILD_TESTS "Build tests (requires vendor/facts)" OFF)
if(CIO_BUILD_TESTS)
  enable_testing()
  add_executable(cio_facts src/cio_facts.c vendor/facts/src/facts.c)
  target_link_libraries(cio_facts PRIVATE cio_static ${UTF8_LIB} m)
  target_include_directories(cio_facts PRIVATE vendor/facts/include ${UTF8_INCLUDE})
  add_test(NAME cio_facts COMMAND cio_facts)
endif()
