///
/// config.hpp
///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_CONFIG_HPP
#define GCE_CONFIG_HPP

#include <boost/config.hpp>
#include <gce/user.hpp>

/// Suppress some vc warnings.
#ifdef BOOST_MSVC
# pragma warning(disable : 4251 4231 4660 4275 4355 4244 4307)
#endif

/// Ensure occupy entire cache(s) line.
#define GCE_CACHE_ALIGNED_VAR(type, var) \
  type var; \
  byte_t pad_##var[(sizeof(type)/GCE_CACHE_LINE_SIZE + 1)*GCE_CACHE_LINE_SIZE - sizeof(type)];

#endif /// GCE_CONFIG_HPP
