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

if (GCE_LUA)
  # Provide user options to customise the build process.
  option (GCE_LUALIB_DEV "GCE.Lualib dev mode" OFF)
  option (GCE_LUALIB_BUILD_EXAMPLE "Build GCE.Lualib examples" OFF)
  option (GCE_LUALIB_BUILD_TEST "Build GCE.Lualib tests" OFF)

  set (LINK_LIBS ${Boost_LIBRARIES} ${Lua_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  if (UNIX)
    set (LINK_LIBS ${LINK_LIBS} rt dl)
  endif ()

  # Not only x64 linux need this, but all linux gcc, we need this.
  if (CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
    add_definitions (-fPIC)
  endif ()

  if (GCE_LUALIB_DEV)
    # Set up all files.
    file (GLOB_RECURSE GCE_LUALIB_HEADER_FILES "${PROJECT_SOURCE_DIR}/gce/lualib/*.hpp")
    file (GLOB_RECURSE GCE_LUALIB_SOURCE_FILES "${PROJECT_SOURCE_DIR}/libs/lualib/src/src.cpp")

    # GCE.Lualib library for dev
    add_library (gce_lualib STATIC ${GCE_LUALIB_SOURCE_FILES} ${GCE_LUALIB_HEADER_FILES})
  endif ()

  # All headers need install include dependences.
  install (
    DIRECTORY ${PROJECT_SOURCE_DIR}/gce/lualib DESTINATION include/gce
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

  file (GLOB GCE_LUALIB_GLOB_DEPEN_FILES "${PROJECT_SOURCE_DIR}/gce/*.hpp")
  install (
    FILES ${GCE_LUALIB_GLOB_DEPEN_FILES} DESTINATION include/gce
    )

  # Build examples.
  if (GCE_LUALIB_BUILD_EXAMPLE)
    add_subdirectory (example)
  endif ()

  # Build tests.
  if (GCE_LUALIB_BUILD_TEST)
    add_subdirectory (test)
  endif ()
endif ()
