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

# Build example - cluster5
set (EXAMPLE_LINK_LIBS ${LINK_LIBS} ${Lua_LIBRARIES})
if (GCE_OPENSSL)
  set(EXAMPLE_LINK_LIBS ${EXAMPLE_LINK_LIBS} ${OpenSSL_LIBRARIES})
endif ()
if (UNIX)
  set (EXAMPLE_LINK_LIBS ${EXAMPLE_LINK_LIBS} dl)
endif ()

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

file (GLOB_RECURSE GCE_EXAMPLE_CLUSTER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file (GLOB_RECURSE GCE_EXAMPLE_CLUSTER_FILES ${GCE_EXAMPLE_CLUSTER_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable (gce_example_cluster5 ${GCE_EXAMPLE_CLUSTER_FILES})

# Copy lua files to build dir

file (GLOB GCE_EXAMPLE_LUA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.lua")
add_custom_command (
  TARGET gce_example_cluster5
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "copy lua files to gce.example.cluster5 build dir..."
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/admin.lua ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/master.lua ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/node.lua ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/node_quit.lua ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/actor/lua/gce.lua ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/actor/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/asio/lua/asio.lua ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/asio/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/example/cluster5
  COMMAND ${CMAKE_COMMAND} -E echo "done."
  VERBATIM
  )
if (${GCE_PACKER} EQUAL ${GCE_ADATA})
  add_custom_command (
    TARGET gce_example_cluster5
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy adata lua files to gce.example.cluster5 build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/adata/lua ${PROJECT_BINARY_DIR}/libs/example/cluster5
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
endif ()

if (GCE_LINK_PROP)
  set_target_properties (gce_example_cluster5 PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (EXAMPLE_LINK_LIBS)
  target_link_libraries (gce_example_cluster5 ${EXAMPLE_LINK_LIBS})
endif ()

install (TARGETS gce_example_cluster5 RUNTIME DESTINATION bin/example/cluster5)

install (FILES ${PROJECT_SOURCE_DIR}/gce/actor/lua/gce.lua DESTINATION bin/example/cluster5)
file (GLOB GCE_ACTOR_LUA_FILES "${PROJECT_SOURCE_DIR}/gce/actor/lua/${GCE_LUADIR}/*.lua")
install (FILES ${GCE_ACTOR_LUA_FILES} DESTINATION bin/example/cluster5)

install (FILES ${PROJECT_SOURCE_DIR}/gce/asio/lua/asio.lua DESTINATION bin/example/cluster5)
file (GLOB GCE_ASIO_LUA_FILES "${PROJECT_SOURCE_DIR}/gce/asio/lua/${GCE_LUADIR}/*.lua")
install (FILES ${GCE_ASIO_LUA_FILES} DESTINATION bin/example/cluster5)

file (GLOB GCE_ADATA_LUA_FILES "${PROJECT_SOURCE_DIR}/gce/adata/lua/*.lua")
install (FILES ${GCE_ADATA_LUA_FILES} DESTINATION bin/example/cluster5)

file (GLOB GCE_EXAMPLE_LUA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.lua")
install (FILES ${GCE_EXAMPLE_LUA_FILES} DESTINATION bin/example/cluster5)
file (GLOB GCE_EXAMPLE_LUA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/${GCE_LUADIR}/*.lua")
install (FILES ${GCE_EXAMPLE_LUA_FILES} DESTINATION bin/example/cluster5)
