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

/// Define winver bal bal...
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
# if !defined(WIN32_LEAN_AND_MEAN)
#   define WIN32_LEAN_AND_MEAN /// For WinSock.h has already been included bal bal(Please include windows.h after this if any)...
# endif
#endif



#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

# ifndef _WIN32_WINNT

#   define _WIN32_WINNT GCE_WINVER

# endif

#endif

/// Suppress some vc warnings.
#ifdef BOOST_MSVC
# pragma warning(disable : 4251 4231 4660 4275 4355 4244 4307)
#endif

#endif /// GCE_CONFIG_HPP
