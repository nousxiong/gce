///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_ERRNO_HPP
#define GCE_REDIS_ERRNO_HPP

#include <gce/redis/config.hpp>

namespace gce
{
namespace redis
{
typedef match_t errno_t;
static errno_t const errno_nil = make_match(u64_nil);

static errno_t make_errno(int e)
{
  return make_match(e);
}
}
}

#endif /// GCE_REDIS_ERRNO_HPP
