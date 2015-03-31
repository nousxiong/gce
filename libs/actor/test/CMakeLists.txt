#
# This file is part of the CMake build system for GCE
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

# Build tests.
set (TESTS_LINK_LIBS ${LINK_LIBS})
file (GLOB_RECURSE GCE_ACTOR_UNIT_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file (GLOB_RECURSE GCE_ACTOR_UNIT_TEST_FILES ${GCE_ACTOR_UNIT_TEST_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable (gce_actor_ut ${GCE_ACTOR_UNIT_TEST_FILES})

# Copy lua files to build dir
if (GCE_LUA)
  if (${GCE_LUA_VERSION} STREQUAL "5.1")
    set (GCE_LUADIR "lua51")
  elseif (${GCE_LUA_VERSION} STREQUAL "5.2")
    set (GCE_LUADIR "lua52")
  elseif (${GCE_LUA_VERSION} STREQUAL "5.3")
    set (GCE_LUADIR "lua53")
  elseif (${GCE_LUA_VERSION} STREQUAL "jit2")
    set (GCE_LUADIR "luajit")
  endif ()
  add_custom_command (
    TARGET gce_actor_ut
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy lua files to build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_lua_actor ${PROJECT_BINARY_DIR}/libs/actor/test/test_lua_actor
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/actor/lua/gce.lua ${PROJECT_BINARY_DIR}/libs/actor/test
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/actor/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/actor/test
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
endif ()

if (GCE_LINK_PROP)
  set_target_properties (gce_actor_ut PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (TESTS_LINK_LIBS)
  target_link_libraries (gce_actor_ut ${TESTS_LINK_LIBS})
endif ()

install (TARGETS gce_actor_ut RUNTIME DESTINATION bin)
if (GCE_LUA)
  install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_lua_actor DESTINATION bin)
endif ()
