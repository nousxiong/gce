#
# This file is part of the CMake build system for Gce
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

if (WIN32)
  if (${CMAKE_GENERATOR} STREQUAL "Visual Studio 11" OR ${CMAKE_GENERATOR} STREQUAL "Visual Studio 12")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
  endif ()
endif ()

# Build tests.
set (TESTS_LINK_LIBS gce_actor ${LINK_LIBS})
file (GLOB_RECURSE GCE_ACTOR_UNIT_TEST_FILES "${PROJECT_SOURCE_DIR}/libs/actor/test/*.cpp")
file (GLOB_RECURSE GCE_ACTOR_UNIT_TEST_FILES ${GCE_ACTOR_UNIT_TEST_FILES} "${PROJECT_SOURCE_DIR}/libs/actor/test/*.hpp")
add_executable (gce_actor_ut ${GCE_ACTOR_UNIT_TEST_FILES})

if (GCE_STATIC)
  set (GCE_LINK_PROP "${GCE_LINK_PROP} -static -static-libgcc -static-libstdc++")
endif ()
if (GCE_LINK_PROP)
  set_target_properties (gce_actor_ut PROPERTIES LINK_FLAGS "${GCE_LINK_PROP}")
endif ()

target_link_libraries (gce_actor_ut ${TESTS_LINK_LIBS})
install (TARGETS gce_actor_ut RUNTIME DESTINATION bin)
