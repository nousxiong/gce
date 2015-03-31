///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_LOG_CONFIG_HPP
#define GCE_LOG_CONFIG_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>

/// Do not define WIN32_LEAN_AND_MEAN before include boost/atomic.hpp
/// Or you will recv:
///   "error C3861: '_InterlockedExchange': identifier not found"
/// Under winxp(x86) vc9
#include <boost/atomic.hpp>

#include <gce/amsg/all.hpp>
#include <boost/chrono.hpp>
#include <boost/function.hpp>

#ifndef GCE_SMALL_LOG_META_SIZE
# define GCE_SMALL_LOG_META_SIZE 32
#endif

#ifndef GCE_LOG_META_MIN_GROW_SIZE
# define GCE_LOG_META_MIN_GROW_SIZE 32
#endif

#ifndef GCE_SMALL_LOG_SIZE
# define GCE_SMALL_LOG_SIZE 128
#endif

#ifndef GCE_LOG_MIN_GROW_SIZE
# define GCE_LOG_MIN_GROW_SIZE 64
#endif

namespace gce
{
namespace log
{
enum level
{
  debug = 0,
  info,
  warn,
  error,
  fatal
};

static const char* const level_string[] =
{
  "DEBUG",
  "INFO",
  "WARN",
  "ERROR",
  "FATAL"
};

inline char const* const to_string(level lv)
{
  return level_string[lv];
}

class record;
typedef boost::function<void (record&)> logger_t;

typedef boost::chrono::system_clock system_clock_t;
typedef system_clock_t::time_point time_point_t;
typedef system_clock_t::duration duration_t;
} /// namespace log
} /// namespace gce

#endif /// GCE_LOG_CONFIG_HPP
