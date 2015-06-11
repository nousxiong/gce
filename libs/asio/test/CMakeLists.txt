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

# Build tests.
set (TESTS_LINK_LIBS ${LINK_LIBS})
if (GCE_OPENSSL)
  set(TESTS_LINK_LIBS ${TESTS_LINK_LIBS} ${OpenSSL_LIBRARIES})
  if (UNIX)
    set (TESTS_LINK_LIBS ${TESTS_LINK_LIBS} dl)
  endif ()
endif ()

file (GLOB_RECURSE GCE_ASIO_UNIT_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file (GLOB_RECURSE GCE_ASIO_UNIT_TEST_FILES ${GCE_ASIO_UNIT_TEST_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable (gce_asio_ut ${GCE_ASIO_UNIT_TEST_FILES})

# Copy lua files to build dir
if (GCE_LUA)
  add_custom_command (
    TARGET gce_asio_ut
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy lua files to gce.asio.test build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_lua_asio ${PROJECT_BINARY_DIR}/libs/asio/test/test_lua_asio
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/actor/lua/gce.lua ${PROJECT_BINARY_DIR}/libs/asio/test
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/actor/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/asio/test
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/asio/lua/asio.lua ${PROJECT_BINARY_DIR}/libs/asio/test
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/asio/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/asio/test
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
  if (${GCE_PACKER} EQUAL ${GCE_ADATA})
    add_custom_command (
      TARGET gce_asio_ut
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "copy adata lua files to gce.asio.test build dir..."
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/adata/lua ${PROJECT_BINARY_DIR}/libs/asio/test
      COMMAND ${CMAKE_COMMAND} -E echo "done."
      VERBATIM
      )
  endif ()
endif ()

# Copy test used openssl files
if (GCE_OPENSSL)
  add_custom_command (
    TARGET gce_asio_ut
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy openssl files to gce.asio.test build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_ssl_asio ${PROJECT_BINARY_DIR}/libs/asio/test/test_ssl_asio
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
endif ()

if (GCE_LINK_PROP)
  set_target_properties (gce_asio_ut PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (TESTS_LINK_LIBS)
  target_link_libraries (gce_asio_ut ${TESTS_LINK_LIBS})
endif ()

install (TARGETS gce_asio_ut RUNTIME DESTINATION bin)
if (GCE_LUA)
  install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_lua_asio DESTINATION bin)
endif ()
if (GCE_OPENSSL)
  install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_ssl_asio DESTINATION bin)
endif ()
