#
# This file is part of the CMake build system for Gce
#
# CMake auto-generated configuration options. 
# Do not check in modified versions of this file.
#
# Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# See https://github.com/nousxiong/gce for latest version.
#

cmake_minimum_required (VERSION 2.8.6 FATAL_ERROR)
project (gce)

# The version number.
set (GCE_VERSION_MAJOR 0)
set (GCE_VERSION_MINOR 8)

# Provide user options to customise the build process.
set (GCE_ASIO_ALLOC_HANDLER_SIZE "1024" CACHE STRING "Boost.Asio handler allocator size")

if (UNIX)
  option (GCE_STATIC "Build gce runtime static" OFF)
endif ()

if (WIN32)
  add_definitions (-D_WIN32_WINNT=0x0501)
endif ()

# Set glibc.
if (GCE_STATIC)
  set (GLIBC_INCLUDEDIR "" CACHE PATH "Path to glibc include directory")
  set (GLIBC_LIBRARYDIR "" CACHE PATH "Path to glibc libraries directory")
  if (GLIBC_INCLUDEDIR)
    include_directories (${GLIBC_INCLUDEDIR})
  endif ()
  if (GLIBC_LIBRARYDIR)
    link_directories (${GLIBC_LIBRARYDIR})
  endif ()
endif ()

# Add the source and build tree to the search path for include gce header files.
include_directories (${PROJECT_SOURCE_DIR})
include_directories (${PROJECT_BINARY_DIR})

# Boost libraries search.
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_MULTITHREADED ON)
if (GCE_STATIC)
  set (Boost_USE_STATIC_RUNTIME ON)
else ()
  set (Boost_USE_STATIC_RUNTIME OFF)
endif ()
if (BOOST_ROOT)
  set (Boost_NO_SYSTEM_PATHS ON)
endif ()

find_package (Boost 1.54.0 REQUIRED atomic coroutine context system timer chrono thread)

mark_as_advanced (Boost_DIR)
if (WIN32)
  mark_as_advanced (Boost_LIB_DIAGNOSTIC_DEFINITIONS)
endif ()

include_directories (${Boost_INCLUDE_DIRS})

# Find threads
find_package (Threads)

set (CMAKE_VERBOSE_MAKEFILE true)

# Select build sub libs.
set (SUB_LIBRARYS "actor" CACHE STRING "Select build sub libs")
string (TOLOWER ${SUB_LIBRARYS} SUB_LIBRARYS_TOLOWER)
string (REGEX MATCHALL "[a-z_]+" SUB_LIB_NAME_LIST ${SUB_LIBRARYS_TOLOWER})

foreach (SUB_LIB_NAME ${SUB_LIB_NAME_LIST})
  add_subdirectory (libs/${SUB_LIB_NAME})
endforeach(SUB_LIB_NAME)

# Configure a header file to pass some of the CMake settings to the source code.
configure_file (
  "${PROJECT_SOURCE_DIR}/user.hpp.in"
  "${PROJECT_BINARY_DIR}/gce_user.hpp"
  )

install (FILES ${PROJECT_BINARY_DIR}/gce_user.hpp DESTINATION include/gce)

# Build a CPack driven installer package.
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${GCE_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${GCE_VERSION_MINOR}")
set (CPACK_PACKAGE_CONTACT "Nous Xiong: 348944179@qq.com")
include (CPack)
