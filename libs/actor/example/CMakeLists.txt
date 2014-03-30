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

# Build examples.
set (EXAMPLES_LINK_LIBS gce_actor ${LINK_LIBS})

if (GCE_STATIC)
  set (GCE_LINK_PROP "${GCE_LINK_PROP} -static -static-libgcc -static-libstdc++")
endif ()

# Gce work with asio.
add_subdirectory (asio)

# Gce cluster.
add_subdirectory (cluster)



# Gce cluster client.
add_subdirectory (cluster_client)



# Helloworld.
add_subdirectory (helloworld)

# Link.
add_subdirectory (link)

# Matching.
add_subdirectory (matching)

# Two actors pingpong.
add_subdirectory (pingpong)

