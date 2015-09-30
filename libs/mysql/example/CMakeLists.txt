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

# Build examples.
set (EXAMPLES_LINK_LIBS ${LINK_LIBS})

if (GCE_STATIC)
  set (GCE_LINK_PROP "${GCE_LINK_PROP} -static -static-libgcc -static-libstdc++")
endif ()
