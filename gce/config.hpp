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
#include <gce/user.hpp>
#include <boost/bind/placeholders.hpp>

/// Suppress some vc warnings.
#if BOOST_COMP_MSVC
# pragma warning(disable : 4251 4231 4660 4275 4355 4244 4307 4996)
#endif

#if BOOST_OS_WINDOWS
# define GCE_IOV_MAX 64 /// suppose >= winnt
#else
# if IOV_MAX < 64
#   define GCE_IOV_MAX IOV_MAX
# else
#   define GCE_IOV_MAX 64
# endif
#endif

/// Ensure occupy entire cache(s) line.
#define GCE_CACHE_ALIGNED_VAR(type_name, var) \
  type_name var; \
  byte_t pad_##var[(sizeof(type_name)/GCE_CACHE_LINE_SIZE + 1)*GCE_CACHE_LINE_SIZE - sizeof(type_name)];

#if defined(GCE_LUA)
# define GCE_SCRIPT
#endif

/// boost placeholders define, compatible with cpp11's std::placeholders
namespace
{
static boost::arg<1> _arg1;
static boost::arg<2> _arg2;
static boost::arg<3> _arg3;
static boost::arg<4> _arg4;
static boost::arg<5> _arg5;
static boost::arg<6> _arg6;
static boost::arg<7> _arg7;
static boost::arg<8> _arg8;
static boost::arg<9> _arg9;
}

#endif /// GCE_CONFIG_HPP
