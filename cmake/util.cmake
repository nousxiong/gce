#
# This file is part of the CMake build system for GCE
#
# Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# See https://github.com/nousxiong/gce for latest version.
#

# Add library.
macro (gce_add_library library_name)
  string (TOUPPER ${library_name} gce_add_library_upper_name)
  set (${gce_add_library_upper_name}_LIBRARYDIR "" CACHE PATH "Path to ${library_name} libraries directory")
  set (${gce_add_library_upper_name}_INCLUDEDIR "" CACHE PATH "Path to ${library_name} include directory")
  include_directories (${${gce_add_library_upper_name}_INCLUDEDIR})
endmacro (gce_add_library)

# Set library.
macro (gce_set_lib library_name name)
  string (TOUPPER ${library_name} gce_set_lib_upper_name)
  string (TOLOWER ${name} gce_set_lib_lower_name)
  find_library (${name}_LIBRARY_DEBUG NAMES ${gce_set_lib_lower_name}_debug PATHS "${${gce_set_lib_upper_name}_LIBRARYDIR}" NO_DEFAULT_PATH)
  find_library (${name}_LIBRARY_RELEASE NAMES ${gce_set_lib_lower_name} PATHS "${${gce_set_lib_upper_name}_LIBRARYDIR}" NO_DEFAULT_PATH)

  if (${name}_LIBRARY_DEBUG AND ${name}_LIBRARY_RELEASE)
    if (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
      set (${name}_LIBRARIES optimized ${${name}_LIBRARY_RELEASE} debug ${${name}_LIBRARY_DEBUG})
    else ()
      set (${name}_LIBRARIES ${${name}_LIBRARY_RELEASE})
    endif ()
  endif ()

  if (${name}_LIBRARY_RELEASE AND NOT ${name}_LIBRARY_DEBUG)
    set (${name}_LIBRARIES ${${name}_LIBRARY_RELEASE})
  endif ()

  if (${name}_LIBRARY_DEBUG AND NOT ${name}_LIBRARY_RELEASE)
    set (${name}_LIBRARIES ${${name}_LIBRARY_DEBUG})
  endif ()
endmacro (gce_set_lib)