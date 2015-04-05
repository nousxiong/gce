///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_NET_OPTION_HPP
#define GCE_ACTOR_NET_OPTION_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/duration.hpp>
#include <gce/actor/net_option.adl.h>

namespace gce
{
typedef adl::net_option netopt_t;

inline netopt_t make_netopt(
  bool is_router = false, /// if is router, set it true
  duration_t heartbeat_period = seconds(30),
  int32_t heartbeat_count = 3,
  duration_t init_reconn_period = seconds(3), /// init conn, between two connects' period
  int32_t init_reconn_try = 2, /// init conn, how many try to reconnect before give up
  duration_t reconn_period = seconds(10), /// in one reconn, between two connects' period
  int32_t reconn_try = 3, /// how many try to reconnect before drop cache msgs
  duration_t rebind_period = seconds(5), /// bind, between two bind' period
  int32_t rebind_try = 3 /// bind, how many try to rebind before give up
  )
{
  netopt_t opt;
  opt.is_router = is_router ? 1 : 0;
  opt.heartbeat_period = heartbeat_period;
  opt.heartbeat_count = heartbeat_count;
  opt.init_reconn_period = init_reconn_period;
  opt.init_reconn_try = init_reconn_try;
  opt.reconn_period = reconn_period;
  opt.reconn_try = reconn_try;
  opt.rebind_period = rebind_period;
  opt.rebind_try = rebind_try;
  return opt;
}
}

#endif /// GCE_ACTOR_NET_OPTION_HPP
