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

namespace gce
{
struct net_option
{
  net_option()
    : is_router_(false)
    , heartbeat_period_(seconds_t(30))
    , heartbeat_count_(3)
    , init_reconn_period_(seconds_t(3))
    , init_reconn_try_(2)
    , reconn_period_(seconds_t(10))
    , reconn_try_(3)
  {
  }

  /// if is router, set it true
  bool is_router_;

  duration_type heartbeat_period_;
  int heartbeat_count_;

  /// init conn, between two connects' period
  duration_type init_reconn_period_;

  /// init conn, how many try to reconnect before give up
  int init_reconn_try_;

  /// in one reconn, between two connects' period
  duration_type reconn_period_;

  /// how many try to reconnect before drop cache msgs
  int reconn_try_;
};
}

#endif /// GCE_ACTOR_NET_OPTION_HPP
