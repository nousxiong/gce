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

file (GLOB_RECURSE GCE_ACTOR_EXAMPLE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file (GLOB_RECURSE GCE_ACTOR_EXAMPLE_FILES ${GCE_ACTOR_EXAMPLE_FILES} "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
add_executable (gce_actor_matching ${GCE_ACTOR_EXAMPLE_FILES})

if (GCE_LINK_PROP)
  set_target_properties (gce_actor_matching PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

if (EXAMPLES_LINK_LIBS)
  target_link_libraries (gce_actor_matching ${EXAMPLES_LINK_LIBS})
endif ()
install (TARGETS gce_actor_matching RUNTIME DESTINATION bin)

