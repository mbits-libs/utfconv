# Copyright (c) 2020 midnightBITS
# This file is licensed under MIT license (see LICENSE for details)

cmake_minimum_required (VERSION 3.12)
project (utfconv
  VERSION 1.0.0
  DESCRIPTION "Conversion library between string, u16string, u32string and u8string"
  LANGUAGES CXX)

set(PROJECT_VERSION_STABILITY "")

if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(STATUS "utfconv: Standalone")
  set(UTFCONV_STANDALONE ON)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Standalone will be compiled with C++20 to suppport char8_t,
  # clients need at least 17 to compile (for string_view for instance).
  set(STANDARD 20 CACHE BOOL "Tweak the required standard")
  set(CMAKE_CXX_STANDARD ${STANDARD})
  set(CMAKE_CXX_STANDARD_REQUIRED OFF)
  set(CMAKE_CXX_EXTENSIONS OFF)

  find_package(Python3 COMPONENTS Interpreter REQUIRED)

  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
  find_package(GTest REQUIRED CONFIG HINTS ${PROJECT_BINARY_DIR})
else()
  message(STATUS "utfconv: Subdir")
  set(UTFCONV_STANDALONE OFF)
endif()

set(UTFCONV_TESTING ${UTFCONV_STANDALONE} CACHE BOOL "Compile and/or run self-tests")
set(UTFCONV_INSTALL ${UTFCONV_STANDALONE} CACHE BOOL "Install the library")

if (UTFCONV_TESTING)
  set(COVERALLS_PREFIX UTFCONV_)
  list(APPEND UTFCONV_COVERALLS_DIRS
    include/utf
    src
  )

  include(tools/coveralls/Coveralls.cmake)
endif()

add_subdirectory(external)

# See <https://github.com/lefticus/cppbestpractices/blob/v1.0.0/02-Use_the_Tools_Available.md#compilers>
if (MSVC)
  set(UTFCONV_ADDITIONAL_WALL_FLAGS
      /permissive-
      /Zc:__cplusplus
      /W4
      /w14242
      /w14254
      /w14263
      /w14265
      /w14287
      /we4289
      /w14296
      /w14311
      /w14545
      /w14546
      /w14547
      /w14549
      /w14555
      /w14619
      /w14640
      /w14826
      /w14905
      /w14906
      /w14928
      /w14946)
else()
  set(UTFCONV_ADDITIONAL_WALL_FLAGS
      -Wall -Wextra
      -Wnon-virtual-dtor
      -Wold-style-cast
      -Wcast-align
      -Wunused
      -Woverloaded-virtual
      -Wpedantic
      -Wconversion
      -Wsign-conversion
      -Wnull-dereference
      -Wdouble-promotion
      -Wformat=2
  )
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    list(APPEND UTFCONV_ADDITIONAL_WALL_FLAGS -fcolor-diagnostics) # -Wlifetime
  else()
    list(APPEND UTFCONV_ADDITIONAL_WALL_FLAGS
      -fdiagnostics-color
      -Wmisleading-indentation
      -Wduplicated-cond
      -Wduplicated-branches
      -Wlogical-op
      -Wuseless-cast
      )
  endif()
endif()

configure_file(src/version.hpp.in include/utf/version.hpp @ONLY)

set(SRCS
  src/utf.cpp
  src/version.cpp
  include/utf/utf.hpp
  "${CMAKE_CURRENT_BINARY_DIR}/include/utf/version.hpp"
)


add_library(${PROJECT_NAME} STATIC ${SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE ${UTFCONV_ADDITIONAL_WALL_FLAGS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)
target_link_libraries(${PROJECT_NAME} PUBLIC semver)
set_target_properties(${PROJECT_NAME} PROPERTIES
  VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

##################################################################
##  INSTALL
##################################################################

if (UTFCONV_INSTALL)
  install(TARGETS ${PROJECT_NAME})
  install(DIRECTORY include/utf DESTINATION include)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/utf/version.hpp" DESTINATION include/utf)
endif()

##################################################################
##  TESTING
##################################################################

if (UTFCONV_TESTING)

set(EXTERNAL_GTEST ON)
include( ${CMAKE_CURRENT_SOURCE_DIR}/tools/testing/googletest.cmake )

add_test_executable(${PROJECT_NAME}-test DATA_PATH data LIBRARIES ${PROJECT_NAME})
target_compile_options(${PROJECT_NAME}-test PRIVATE ${UTFCONV_ADDITIONAL_WALL_FLAGS})
target_compile_features(${PROJECT_NAME}-test PRIVATE cxx_std_17)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}-test)
endif()
