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

if (GCE_REDIS)
  # Provide user options to customise the build process.
  option (GCE_REDIS_DEV "GCE.Redis dev mode" OFF)
  option (GCE_REDIS_BUILD_EXAMPLE "Build GCE.Redis examples" OFF)
  option (GCE_REDIS_BUILD_TEST "Build GCE.Redis tests" OFF)
  
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
  
  if (GCE_REDIS_DEV)
    # Set up all files.
    file (GLOB_RECURSE GCE_REDIS_HEADER_FILES "${PROJECT_SOURCE_DIR}/gce/redis/*.hpp")
    if (GCE_LUA)
      file (GLOB GCE_REDIS_HEADER_FILES ${GCE_REDIS_HEADER_FILES} "${PROJECT_SOURCE_DIR}/gce/redis/*.h")
    endif ()
    file (GLOB_RECURSE GCE_REDIS_SOURCE_FILES "${PROJECT_SOURCE_DIR}/libs/redis/src/src.cpp")
  
    # GCE.Redis library for dev
    add_library (gce_redis STATIC ${GCE_REDIS_SOURCE_FILES} ${GCE_REDIS_HEADER_FILES})
  endif ()
  
  # All headers need install include dependences.
  install (
    DIRECTORY ${PROJECT_SOURCE_DIR}/gce/redis DESTINATION include/gce
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
    
  install (
    DIRECTORY ${PROJECT_SOURCE_DIR}/gce/resp DESTINATION include/gce
    PATTERN "impl" EXCLUDE
    PATTERN "CVS" EXCLUDE
    PATTERN ".svn" EXCLUDE
    PATTERN "lua" EXCLUDE
    )
  
  file (GLOB GCE_REDIS_GLOB_DEPEN_FILES "${PROJECT_SOURCE_DIR}/gce/*.hpp")
  install (
    FILES ${GCE_REDIS_GLOB_DEPEN_FILES} DESTINATION include/gce
    )
  
  if (GCE_STATIC)
    set (GCE_LINK_PROP "${GCE_LINK_PROP} -static -static-libgcc -static-libstdc++")
  endif ()
  
  # Build examples.
  if (GCE_REDIS_BUILD_EXAMPLE)
    add_subdirectory (example)
  endif ()
  
  # Build tests.
  if (GCE_REDIS_BUILD_TEST)
    add_subdirectory (test)
  endif ()
  
  if (GCE_LUA)
    install (FILES ${PROJECT_SOURCE_DIR}/gce/redis/lua/redis.lua DESTINATION bin)
    
    file (GLOB GCE_REDIS_LUA_FILES "${PROJECT_SOURCE_DIR}/gce/redis/lua/${GCE_LUADIR}/*.lua")
    install (FILES ${GCE_REDIS_LUA_FILES} DESTINATION bin)
  endif ()
endif ()
