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
option (GCE_HTTP_DEV "GCE.Http dev mode" OFF)
option (GCE_HTTP_BUILD_EXAMPLE "Build GCE.Http examples" OFF)
option (GCE_HTTP_BUILD_TEST "Build GCE.Http tests" OFF)

set (LINK_LIBS ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
if (UNIX)
  set (LINK_LIBS ${LINK_LIBS} rt)
endif ()
if (GCE_LUA)
  set(LINK_LIBS ${LINK_LIBS} ${Lua_LIBRARIES})
  if (UNIX)
    set (LINK_LIBS ${LINK_LIBS} dl)
  endif ()
endif ()

# Not only x64 linux need this, but all linux gcc, we need this.
if (CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
  add_definitions (-fPIC)
endif ()

if (GCE_HTTP_DEV)
  # Set up all files.
  file (GLOB_RECURSE GCE_HTTP_HEADER_FILES "${PROJECT_SOURCE_DIR}/gce/http/*.hpp")
  if (GCE_LUA)
    file (GLOB GCE_HTTP_HEADER_FILES ${GCE_HTTP_HEADER_FILES} "${PROJECT_SOURCE_DIR}/gce/http/*.h")
  endif ()
  file (GLOB_RECURSE GCE_HTTP_SOURCE_FILES "${PROJECT_SOURCE_DIR}/libs/http/src/src.cpp")

  # GCE.Http library for dev
  add_library (gce_http STATIC ${GCE_HTTP_SOURCE_FILES} ${GCE_HTTP_HEADER_FILES})
endif ()

# All headers need install include dependences.
install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/http DESTINATION include/gce
  PATTERN "impl" EXCLUDE
  PATTERN "CVS" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN "lua" EXCLUDE
  )

install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/detail DESTINATION include/gce
  PATTERN "impl" EXCLUDE
  PATTERN "CVS" EXCLUDE
  PATTERN ".svn" EXCLUDE
  )

file (GLOB GCE_HTTP_GLOB_DEPEN_FILES "${PROJECT_SOURCE_DIR}/gce/*.hpp")
install (
  FILES ${GCE_HTTP_GLOB_DEPEN_FILES} DESTINATION include/gce
  )

if (GCE_STATIC)
  set (GCE_LINK_PROP "${GCE_LINK_PROP} -static -static-libgcc -static-libstdc++")
endif ()

# Build examples.
if (GCE_HTTP_BUILD_EXAMPLE)
  add_subdirectory (example)
endif ()

# Build tests.
if (GCE_HTTP_BUILD_TEST)
  add_subdirectory (test)
endif ()

if (GCE_LUA)
  install (FILES ${PROJECT_SOURCE_DIR}/gce/http/lua/http.lua DESTINATION bin)
  
  file (GLOB GCE_HTTP_LUA_FILES "${PROJECT_SOURCE_DIR}/gce/http/lua/${GCE_LUADIR}/*.lua")
  install (FILES ${GCE_HTTP_LUA_FILES} DESTINATION bin)
endif ()
