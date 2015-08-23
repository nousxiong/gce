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

# Build example - cluster3
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

add_executable (gce_example_cluster3 ${GCE_EXAMPLE_CLUSTER_FILES})

if (GCE_LINK_PROP)
  set_target_properties (gce_example_cluster3 PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (EXAMPLE_LINK_LIBS)
  target_link_libraries (gce_example_cluster3 ${EXAMPLE_LINK_LIBS})
endif ()

install (TARGETS gce_example_cluster3 RUNTIME DESTINATION bin)
