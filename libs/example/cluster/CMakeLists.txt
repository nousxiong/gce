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

# Build example - cluster
set (EXAMPLE_LINK_LIBS ${LINK_LIBS})
if (GCE_LUA OR GCE_OPENSSL)
  if (GCE_LUA)
    set(EXAMPLE_LINK_LIBS ${EXAMPLE_LINK_LIBS} ${Lua_LIBRARIES})
  endif ()
  if (GCE_OPENSSL)
    set(EXAMPLE_LINK_LIBS ${EXAMPLE_LINK_LIBS} ${OpenSSL_LIBRARIES})
  endif ()
  if (UNIX)
    set (EXAMPLE_LINK_LIBS ${EXAMPLE_LINK_LIBS} dl)
  endif ()
endif ()

file (GLOB_RECURSE GCE_EXAMPLE_CLUSTER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file (GLOB_RECURSE GCE_EXAMPLE_CLUSTER_FILES ${GCE_EXAMPLE_CLUSTER_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable (gce_example_cluster ${GCE_EXAMPLE_CLUSTER_FILES})

# Copy lua files to build dir
if (GCE_LUA)
  add_custom_command (
    TARGET gce_example_cluster
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy lua files to gce.example.cluster build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/cluster_lua_example ${PROJECT_BINARY_DIR}/libs/example/cluster/cluster_lua_example
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/actor/lua/gce.lua ${PROJECT_BINARY_DIR}/libs/example/cluster
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/actor/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/example/cluster
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/asio/lua/asio.lua ${PROJECT_BINARY_DIR}/libs/example/cluster
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/asio/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/example/cluster
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
  if (${GCE_PACKER} EQUAL ${GCE_ADATA})
    add_custom_command (
      TARGET gce_example_cluster
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "copy adata lua files to gce.example.cluster build dir..."
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/adata/lua ${PROJECT_BINARY_DIR}/libs/example/cluster
      COMMAND ${CMAKE_COMMAND} -E echo "done."
      VERBATIM
      )
  endif ()
endif ()

# Copy test used openssl files
if (GCE_OPENSSL)
  add_custom_command (
    TARGET gce_example_cluster
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy openssl files to gce.example.cluster build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/cluster_ssl_example ${PROJECT_BINARY_DIR}/libs/asio/test/cluster_ssl_example
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
endif ()

if (GCE_LINK_PROP)
  set_target_properties (gce_example_cluster PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (EXAMPLE_LINK_LIBS)
  target_link_libraries (gce_example_cluster ${EXAMPLE_LINK_LIBS})
endif ()

install (TARGETS gce_example_cluster RUNTIME DESTINATION bin)
if (GCE_LUA)
  install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cluster_lua_example DESTINATION bin)
endif ()
if (GCE_OPENSSL)
  install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cluster_ssl_example DESTINATION bin)
endif ()
