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

# Provide user options to customise the build process.
option (GCE_LOG_DEV "GCE.Log dev mode" OFF)
option (GCE_LOG_BUILD_EXAMPLE "Build GCE.Log examples" OFF)
option (GCE_LOG_BUILD_TEST "Build GCE.Log tests" OFF)

set (LINK_LIBS ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
  set (LINK_LIBS ${LINK_LIBS} rt)
endif ()

# Not only x64 linux need this, but all linux gcc, we need this.
if (CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
  add_definitions (-fPIC)
endif ()

if (GCE_LOG_DEV)
  # Set up all files.
  file (GLOB_RECURSE GCE_LOG_HEADER_FILES "${PROJECT_SOURCE_DIR}/gce/log/*.hpp")
  file (GLOB_RECURSE GCE_LOG_SOURCE_FILES "${PROJECT_SOURCE_DIR}/libs/log/src/src.cpp")

  # GCE.Log library for dev
  add_library (gce_log STATIC ${GCE_LOG_SOURCE_FILES} ${GCE_LOG_HEADER_FILES})
endif ()

# All headers need install include dependences.
install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/log DESTINATION include/gce
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

file (GLOB GCE_LOG_GLOB_DEPEN_FILES "${PROJECT_SOURCE_DIR}/gce/*.hpp")
install (
  FILES ${GCE_LOG_GLOB_DEPEN_FILES} DESTINATION include/gce
  )

# Build examples.
if (GCE_LOG_BUILD_EXAMPLE)
  add_subdirectory (example)
endif ()

# Build tests.
if (GCE_LOG_BUILD_TEST)
  add_subdirectory (test)
endif ()
