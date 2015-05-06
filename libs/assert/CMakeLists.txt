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
option (GCE_ASSERT_DEV "GCE.Assert dev mode" OFF)
option (GCE_ENABLE_ASSERT "GCE.Assert explicit define assert" OFF)
option (GCE_ASSERT_BUILD_EXAMPLE "Build GCE.Assert examples" OFF)
option (GCE_ASSERT_BUILD_TEST "Build GCE.Assert tests" OFF)

set (LINK_LIBS ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
  set (LINK_LIBS ${LINK_LIBS} rt)
endif ()

# Not only x64 linux need this, but all linux gcc, we need this.
if (CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
  add_definitions (-fPIC)
endif ()

if (GCE_ENABLE_ASSERT)
  add_definitions (-DGCE_ENABLE_ASSERT)
endif ()

if (GCE_ASSERT_DEV)
  # Set up all files.
  file (GLOB_RECURSE GCE_ASSERT_HEADER_FILES "${PROJECT_SOURCE_DIR}/gce/assert/*.hpp")
  file (GLOB_RECURSE GCE_ASSERT_SOURCE_FILES "${PROJECT_SOURCE_DIR}/libs/assert/src/src.cpp")

  # GCE.Assert library for dev
  add_library (gce_assert STATIC ${GCE_ASSERT_SOURCE_FILES} ${GCE_ASSERT_HEADER_FILES})
endif ()

# All headers need install include dependences.
install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/assert DESTINATION include/gce
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

file (GLOB GCE_ASSERT_GLOB_DEPEN_FILES "${PROJECT_SOURCE_DIR}/gce/*.hpp")
install (
  FILES ${GCE_ASSERT_GLOB_DEPEN_FILES} DESTINATION include/gce
  )

# Build examples.
if (GCE_ASSERT_BUILD_EXAMPLE)
  add_subdirectory (example)
endif ()

# Build tests.
if (GCE_ASSERT_BUILD_TEST)
  add_subdirectory (test)
endif ()
