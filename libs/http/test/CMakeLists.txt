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

file (GLOB_RECURSE GCE_HTTP_UNIT_TEST_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file (GLOB_RECURSE GCE_HTTP_UNIT_TEST_FILES ${GCE_HTTP_UNIT_TEST_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

add_executable (gce_http_ut ${GCE_HTTP_UNIT_TEST_FILES})

if (GCE_LINK_PROP)
  set_target_properties (gce_http_ut PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (TESTS_LINK_LIBS)
  target_link_libraries (gce_http_ut ${TESTS_LINK_LIBS})
endif ()

install (TARGETS gce_http_ut RUNTIME DESTINATION bin)
