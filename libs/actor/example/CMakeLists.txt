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

# GCE work with asio.
add_subdirectory (asio)

# GCE cluster.
add_subdirectory (cluster)



# GCE cluster client.
add_subdirectory (cluster_client)



# Helloworld.
add_subdirectory (helloworld)

# Link.
add_subdirectory (link)

# Matching.
add_subdirectory (matching)

# Two actors pingpong.
add_subdirectory (pingpong)

