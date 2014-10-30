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

#include <boost/predef.h>
#include <boost/type_traits/remove_reference.hpp>

/// The configured options and settings for gce.
#define GCE_VERSION_MAJOR 1
#define GCE_VERSION_MINOR 2

/// User options.
#ifndef GCE_ASIO_ALLOC_HANDLER_SIZE
# define GCE_ASIO_ALLOC_HANDLER_SIZE 1024
#endif

#ifdef BOOST_OS_WINDOWS
# ifndef GCE_WINVER
#   define GCE_WINVER 0x0501
# endif
#endif

/// Suppress some vc warnings.
#ifdef BOOST_COMP_MSVC
# pragma warning(disable : 4251 4231 4660 4275 4355 4244 4307 4996)
#endif

/// Ensure occupy entire cache(s) line.
#define GCE_CACHE_ALIGNED_VAR(type_name, var) \
  type_name var; \
  byte_t pad_##var[(sizeof(type_name)/GCE_CACHE_LINE_SIZE + 1)*GCE_CACHE_LINE_SIZE - sizeof(type_name)];

#if defined(GCE_LUA)
# define GCE_HAS_SCRIPT
#endif

#endif /// GCE_CONFIG_HPP
