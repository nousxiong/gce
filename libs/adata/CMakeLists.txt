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

# All headers need install include dependences.
install (
  DIRECTORY ${PROJECT_SOURCE_DIR}/gce/adata DESTINATION include/gce
  PATTERN "impl" EXCLUDE
  PATTERN "CVS" EXCLUDE
  PATTERN ".svn" EXCLUDE
  PATTERN "lua" EXCLUDE
  )

if (GCE_LUA)
  file (GLOB GCE_ADATA_LUA_FILES "${PROJECT_SOURCE_DIR}/gce/adata/lua/*.lua")
  install (FILES ${GCE_ADATA_LUA_FILES} DESTINATION bin)
endif ()
