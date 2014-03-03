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
    , reconn_period_(15)
  {
  }

  seconds_t heartbeat_period_;
  std::size_t heartbeat_count_;
  seconds_t reconn_period_; /// in one reconn, between two connects' period
};
}

#endif /// GCE_ACTOR_NET_OPTION_HPP
