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
file (GLOB_RECURSE GCE_MYSQL_UNIT_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file (GLOB_RECURSE GCE_MYSQL_UNIT_TEST_FILES ${GCE_MYSQL_UNIT_TEST_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable (gce_mysql_ut ${GCE_MYSQL_UNIT_TEST_FILES})

if (MSVC)
  add_custom_command (
    TARGET gce_mysql_ut
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy libmysql.dll to gce.mysql.test build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy ${MYSQLCLIENT_LIBRARYDIR}/libmysql.dll ${PROJECT_BINARY_DIR}/libs/mysql/test
    COMMAND ${CMAKE_COMMAND} -E copy ${MYSQLCLIENT_LIBRARYDIR}/libmysql_debug.dll ${PROJECT_BINARY_DIR}/libs/mysql/test
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
endif ()

# Copy lua files to build dir
if (GCE_LUA)
  add_custom_command (
    TARGET gce_mysql_ut
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "copy lua files to gce.mysql.test build dir..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_lua_mysql ${PROJECT_BINARY_DIR}/libs/mysql/test/test_lua_mysql
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/actor/lua/gce.lua ${PROJECT_BINARY_DIR}/libs/mysql/test
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/actor/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/mysql/test
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/gce/mysql/lua/mysql.lua ${PROJECT_BINARY_DIR}/libs/mysql/test
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/mysql/lua/${GCE_LUADIR} ${PROJECT_BINARY_DIR}/libs/mysql/test
    COMMAND ${CMAKE_COMMAND} -E echo "done."
    VERBATIM
    )
  if (${GCE_PACKER} EQUAL ${GCE_ADATA})
    add_custom_command (
      TARGET gce_mysql_ut
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E echo "copy adata lua files to gce.mysql.test build dir..."
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/gce/adata/lua ${PROJECT_BINARY_DIR}/libs/mysql/test
      COMMAND ${CMAKE_COMMAND} -E echo "done."
      VERBATIM
      )
  endif ()
endif ()

if (GCE_LINK_PROP)
  set_target_properties (gce_mysql_ut PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (TESTS_LINK_LIBS)
  target_link_libraries (gce_mysql_ut ${TESTS_LINK_LIBS})
endif ()

install (TARGETS gce_mysql_ut RUNTIME DESTINATION bin)
if (GCE_LUA)
  install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test_lua_mysql DESTINATION bin)
endif ()
