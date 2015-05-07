#
# This file is part of the CMake build system for GCE
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
set (GCE_VERSION_MAJOR 1)
set (GCE_VERSION_MINOR 2)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include (util)

# Provide user options to customise the build process.
option (GCE_LUA "Allow gce using lua actor" OFF)
option (GCE_OPENSSL "Allow gce using openssl" OFF)
set (GCE_ASIO_ALLOC_HANDLER_SIZE "1024" CACHE STRING "Boost.Asio handler allocator size")
set (GCE_PACKER "amsg" CACHE STRING "GCE pack lib type, option are: adata amsg")

if (GCE_LUA)
  set (GCE_LUA_VERSION "5.1" CACHE STRING "GCE lua version, option are: 5.1 5.2 5.3")
  set (GCE_LUA51 0)
  set (GCE_LUA52 1)
  set (GCE_LUA53 2)
  if (${GCE_LUA_VERSION} STREQUAL "5.1")
    set (GCE_LUADIR "lua51")
    set (GCE_LUA_VERSION ${GCE_LUA51})
  elseif (${GCE_LUA_VERSION} STREQUAL "5.2")
    set (GCE_LUADIR "lua52")
    set (GCE_LUA_VERSION ${GCE_LUA52})
  elseif (${GCE_LUA_VERSION} STREQUAL "5.3")
    set (GCE_LUADIR "lua53")
    set (GCE_LUA_VERSION ${GCE_LUA53})
  else ()
    message (FATAL_ERROR "GCE_LUA_VERSION error, must be 5.1 5.2 5.3 one of them")
  endif ()
endif ()

# Define gce packer types
set (GCE_AMSG 0)
set (GCE_ADATA 1)
if (${GCE_PACKER} STREQUAL "amsg")
  set (GCE_PACKER ${GCE_AMSG})
elseif (${GCE_PACKER} STREQUAL "adata")
  set (GCE_PACKER ${GCE_ADATA})
else ()
  message (FATAL_ERROR "GCE_PACKER error, must be adata or amsg")
endif ()

if (UNIX)
  option (GCE_STATIC "Build gce runtime static" OFF)
endif ()

if (WIN32)
  set (GCE_WINVER "0x0501" CACHE STRING "Windows version maro. Default is 0x0501 - winxp, user can reset")
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

# Set lua
if (GCE_LUA)
  gce_add_library (lua)
  gce_set_lib (lua Lua)
endif ()

# Find openssl
if (GCE_OPENSSL)
  gce_add_library (OpenSSL)
  if (WIN32)
    set (OPENSSL_LIBSSL_NAME "ssleay32" CACHE PATH "Lib file name to openssl ssl name, e.g. ssleay32")
    set (OPENSSL_LIBCRYPTO_NAME "libeay32" CACHE PATH "Lib file name to openssl crypto name, e.g. libeay32")
  else ()
    set (OPENSSL_LIBSSL_NAME "ssl" CACHE PATH "Lib file name to openssl ssl name, e.g. ssl")
    set (OPENSSL_LIBCRYPTO_NAME "crypto" CACHE PATH "Lib file name to openssl crypto name, e.g. crypto")
  endif ()
  gce_set_lib (OpenSSL ${OPENSSL_LIBSSL_NAME})
  gce_set_lib (OpenSSL ${OPENSSL_LIBCRYPTO_NAME})
  set (OpenSSL_LIBRARIES ${${OPENSSL_LIBSSL_NAME}_LIBRARIES} ${${OPENSSL_LIBCRYPTO_NAME}_LIBRARIES})
endif ()

# Add the source and build tree to the search path for include gce header files.
include_directories (${PROJECT_SOURCE_DIR})
include_directories (${PROJECT_BINARY_DIR})

# Boost libraries search.
set (BOOST_ROOT "" CACHE PATH "Path to boost directory, e.g. boost_1_57_0")
set (GCE_BOOST_VERSION "1.57.0" CACHE STRING "Boost version, e.g. 1.57.0")

if (${GCE_BOOST_VERSION} LESS 1.57.0)
  message (FATAL_ERROR "Please using boost 1.57.0 or higher")
endif ()

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

find_package (Boost ${GCE_BOOST_VERSION} REQUIRED atomic coroutine context system regex date_time timer chrono thread)

mark_as_advanced (Boost_DIR)
if (WIN32)
  mark_as_advanced (Boost_LIB_DIAGNOSTIC_DEFINITIONS)
endif ()

include_directories (${Boost_INCLUDE_DIRS})

# Find threads
find_package (Threads)

set (CMAKE_VERBOSE_MAKEFILE true)

if (WIN32)
  # Set >= vc10 SAFESEH to NO
  if (${CMAKE_GENERATOR} MATCHES "Visual Studio 1*")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
  endif ()

  # Set vc bigobj
  if (${CMAKE_GENERATOR} MATCHES "Visual Studio *")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  endif ()
endif ()

# Select build sub libs.
set (SUB_LIBRARYS "actor amsg adata asio log assert" CACHE STRING "Select build sub libs")
string (TOLOWER ${SUB_LIBRARYS} SUB_LIBRARYS_TOLOWER)
string (REGEX MATCHALL "[a-z_]+" SUB_LIB_NAME_LIST ${SUB_LIBRARYS_TOLOWER})

foreach (SUB_LIB_NAME ${SUB_LIB_NAME_LIST})
  add_subdirectory (libs/${SUB_LIB_NAME})
endforeach(SUB_LIB_NAME)

# Configure a header file to pass some of the CMake settings to the source code.
configure_file (
  "${PROJECT_SOURCE_DIR}/user.hpp.in"
  "${PROJECT_BINARY_DIR}/gce/user.hpp"
  )

install (FILES ${PROJECT_BINARY_DIR}/gce/user.hpp DESTINATION include/gce)

# Build a CPack driven installer package.
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${GCE_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${GCE_VERSION_MINOR}")
set (CPACK_PACKAGE_CONTACT "Nous Xiong: 348944179@qq.com")
include (CPack)
