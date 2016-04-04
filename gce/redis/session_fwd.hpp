///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_SESSION_FWD_HPP
#define GCE_REDIS_SESSION_FWD_HPP

#include <gce/redis/config.hpp>

namespace gce
{
namespace redis
{
typedef match_t snid_t;
static snid_t const snid_nil = make_match(u64_nil);

static match_t const sn_open = atom("sn_open");
static match_t const sn_ping = atom("sn_ping");
static match_t const sn_query = atom("sn_query");
static match_t const sn_pubmsg = atom("sn_pubmsg");
}
}

#endif /// GCE_REDIS_SESSION_FWD_HPP
