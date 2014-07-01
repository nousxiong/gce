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

namespace gce
{
struct net_option
{
  net_option()
    : heartbeat_period_(30)
    , heartbeat_count_(3)
    , init_reconn_period_(3)
    , init_reconn_try_(2)
    , reconn_period_(10)
    , reconn_try_(3)
  {
  }

  seconds_t heartbeat_period_;
  std::size_t heartbeat_count_;
  seconds_t init_reconn_period_; /// init conn, between two connects' period
  std::size_t init_reconn_try_; /// init conn, how many try to reconnect before give up
  seconds_t reconn_period_; /// in one reconn, between two connects' period
  std::size_t reconn_try_; /// how many try to reconnect before drop cache msgs
};
}

#endif /// GCE_ACTOR_NET_OPTION_HPP
