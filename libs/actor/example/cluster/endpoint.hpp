///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_ENDPOINT_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_ENDPOINT_HPP

#include <gce/actor/all.hpp>

class endpoint
{
public:
  endpoint()
    : group_index_(gce::u32_nil)
    , sid_(gce::u32_nil)
  {
  }

  endpoint(boost::uint32_t group_index, boost::uint32_t sid)
    : group_index_(group_index)
    , sid_(sid)
  {
  }

  ~endpoint()
  {
  }

public:
  bool valid() const
  {
    return sid_ != gce::u32_nil;
  }

  bool operator==(endpoint const& rhs) const
  {
    return sid_ == rhs.sid_;
  }

  bool operator!=(endpoint const& rhs) const
  {
    return sid_ != rhs.sid_;
  }

  bool operator<(endpoint const& rhs) const
  {
    return sid_ < rhs.sid_;
  }

  boost::uint32_t get_group_index() const
  {
    return group_index_;
  }

  boost::uint32_t get_session_id() const
  {
    return sid_;
  }

private:
  boost::uint32_t group_index_;
  boost::uint32_t sid_;
};

gce::message& operator<<(gce::message& m, endpoint const& src)
{
  m << src.get_group_index() << src.get_session_id();
  return m;
}

gce::message& operator>>(gce::message& msg, endpoint& des)
{
  boost::uint32_t group_index;
  boost::uint32_t sid;
  msg >> group_index >> sid;
  des = endpoint(group_index, sid);
  return msg;
}

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_ENDPOINT_HPP
