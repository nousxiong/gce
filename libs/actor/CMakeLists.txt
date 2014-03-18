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

# Provide user options to customise the build process.
set (GCE_CACHE_LINE_SIZE "64" CACHE STRING "Cache line size")
set (GCE_FREE_CACHE_SIZE "8" CACHE STRING "Free-queue cache size")
set (GCE_SOCKET_RECV_CACHE_SIZE "65535" CACHE STRING "Socket recv cache size")
set (GCE_SOCKET_RECV_MAX_SIZE "60000" CACHE STRING "Socket max recv size")
set (GCE_SMALL_MSG_SIZE "128" CACHE STRING "Small message size")
set (GCE_MSG_MIN_GROW_SIZE "64" CACHE STRING "Message grow min size")
set (GCE_DEFAULT_REQUEST_TIMEOUT_SEC "180" CACHE STRING "Default request timeout seconds, 180 secs")

option (GCE_ACTOR_BUILD_EXAMPLE "Build Gce.Actor examples" ON)
option (GCE_ACTOR_BUILD_TEST "Build Gce.Actor tests" ON)

set (LINK_LIBS ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
  set (LINK_LIBS ${LINK_LIBS} rt)
endif ()

# Not only x64 linux need this, but all linux gcc, we need this.
if (CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
  add_definitions (-fPIC)
endif ()

# Set up all files.
file (GLOB_RECURSE GCE_ACTOR_HEADER_FILES "${PROJECT_SOURCE_DIR}/gce/actor/*.hpp")
file (GLOB_RECURSE GCE_ACTOR_SOURCE_FILES "${PROJECT_SOURCE_DIR}/libs/actor/src/*.cpp")

# Gce.Actor library.
add_library (gce_actor STATIC ${GCE_ACTOR_SOURCE_FILES} ${GCE_ACTOR_HEADER_FILES})

install (
  TARGETS gce_actor
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )

# All headers need install include dependences.
install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/actor DESTINATION include/gce
  PATTERN "impl" EXCLUDE
  PATTERN "CVS" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )

install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/detail DESTINATION include/gce
  PATTERN "impl" EXCLUDE
  PATTERN "CVS" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )

file (GLOB GCE_ACTOR_GLOB_DEPEN_FILES "${PROJECT_SOURCE_DIR}/gce/*.hpp")
install (
  FILES ${GCE_ACTOR_GLOB_DEPEN_FILES} DESTINATION include/gce
  )

# Build examples.
if (GCE_ACTOR_BUILD_EXAMPLE)
  add_subdirectory (example)
endif ()

# Build tests.
if (GCE_ACTOR_BUILD_TEST)
  add_subdirectory (test)
endif ()

# Configure a header file to pass some of the CMake settings to the source code.
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/user.hpp.in"
  "${PROJECT_BINARY_DIR}/gce/actor/user.hpp"
  )

install (FILES ${PROJECT_BINARY_DIR}/gce/actor/user.hpp DESTINATION include/gce/actor)
