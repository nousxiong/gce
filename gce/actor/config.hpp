///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_CONFIG_HPP
#define GCE_ACTOR_CONFIG_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <gce/assert/all.hpp>
#include <gce/log/all.hpp>

/// Do not define WIN32_LEAN_AND_MEAN before include boost/atomic.hpp
/// Or you will recv:
///   "error C3861: '_InterlockedExchange': identifier not found"
/// Under winxp(x86) vc9
#include <boost/atomic.hpp>

#ifndef GCE_PACK
# define GCE_PACK AMSG
#endif

#ifndef GCE_CACHE_LINE_SIZE
# define GCE_CACHE_LINE_SIZE 64
#endif

#ifndef GCE_SOCKET_BIG_MSG_SCALE
# define GCE_SOCKET_BIG_MSG_SCALE 1000
#endif

#endif /// GCE_ACTOR_CONFIG_HPP
