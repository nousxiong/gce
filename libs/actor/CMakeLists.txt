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
option (GCE_ACTOR_DEV "GCE.Actor dev mode" OFF)
option (GCE_ACTOR_BUILD_EXAMPLE "Build GCE.Actor examples" OFF)
option (GCE_ACTOR_BUILD_TEST "Build GCE.Actor tests" OFF)

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

if (GCE_ACTOR_DEV)
  # Set up all files.
  file (GLOB_RECURSE GCE_ACTOR_HEADER_FILES "${PROJECT_SOURCE_DIR}/gce/actor/*.hpp")
  if (GCE_LUA)
    file (GLOB GCE_ACTOR_HEADER_FILES ${GCE_ACTOR_HEADER_FILES} "${PROJECT_SOURCE_DIR}/gce/actor/*.h")
  endif ()
  file (GLOB_RECURSE GCE_ACTOR_SOURCE_FILES "${PROJECT_SOURCE_DIR}/libs/actor/src/src.cpp")

  # GCE.Actor library for dev
  add_library (gce_actor STATIC ${GCE_ACTOR_SOURCE_FILES} ${GCE_ACTOR_HEADER_FILES})
endif ()

# All headers need install include dependences.
install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/actor DESTINATION include/gce
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

file (GLOB GCE_ACTOR_GLOB_DEPEN_FILES "${PROJECT_SOURCE_DIR}/gce/*.hpp")
install (
  FILES ${GCE_ACTOR_GLOB_DEPEN_FILES} DESTINATION include/gce
  )

if (GCE_STATIC)
  set (GCE_LINK_PROP "${GCE_LINK_PROP} -static -static-libgcc -static-libstdc++")
endif ()

# Build examples.
if (GCE_ACTOR_BUILD_EXAMPLE)
  add_subdirectory (example)
endif ()

# Build tests.
if (GCE_ACTOR_BUILD_TEST)
  add_subdirectory (test)
endif ()

if (GCE_LUA)
  install (FILES ${PROJECT_SOURCE_DIR}/gce/actor/lua/gce.lua DESTINATION bin)
  
  file (GLOB GCE_ACTOR_LUA_FILES "${PROJECT_SOURCE_DIR}/gce/actor/lua/${GCE_LUADIR}/*.lua")
  install (FILES ${GCE_ACTOR_LUA_FILES} DESTINATION bin)
endif ()
