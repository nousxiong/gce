///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_CONTEXT_ID_HPP
#define GCE_MYSQL_CONTEXT_ID_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/context_id.adl.h>

namespace gce
{
namespace mysql
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

GCE_PACK(gce::mysql::ctxid_t, (v.ptr_));

#endif /// GCE_MYSQL_CONTEXT_ID_HPP
