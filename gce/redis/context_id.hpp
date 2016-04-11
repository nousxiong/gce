///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_CONTEXT_ID_HPP
#define GCE_REDIS_CONTEXT_ID_HPP

#include <gce/redis/config.hpp>
#include <gce/redis/context_id.adl.h>

namespace gce
{
namespace redis
{
typedef adl::context_id ctxid_t;
class context;
static ctxid_t make_ctxid(context* ctx = 0)
{
  ctxid_t ctxid;
  ctxid.ptr_ = (uint64_t)ctx;
  return ctxid;
}

static context* get_context(ctxid_t const& ctxid)
{
  return (context*)ctxid.ptr_;
}
}
}

GCE_PACK(gce::redis::ctxid_t, (v.ptr_));

#endif /// GCE_REDIS_CONTEXT_ID_HPP
