///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DURATION_HPP
#define GCE_ACTOR_DURATION_HPP

#include <gce/actor/config.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace gce
{
struct duration_type
{
  enum type
  {
    raw = 0,
    millisec,
    second,
    minute,
    hour
  };

  duration_type()
    : dur_(duration_t::zero())
    , ty_(raw)
  {
  }

  duration_type(duration_t d)
    : dur_(d)
    , ty_(raw)
  {
  }

  duration_type(millisecs_t d)
    : dur_(d)
    , ty_(millisec)
  {
  }

  duration_type(seconds_t d)
    : dur_(d)
    , ty_(second)
  {
  }

  duration_type(minutes_t d)
    : dur_(d)
    , ty_(minute)
  {
  }

  duration_type(hours_t d)
    : dur_(d)
    , ty_(hour)
  {
  }

  operator duration_t() const
  {
    return dur_;
  }

  duration_type& operator=(duration_t d)
  {
    dur_ = d;
    return *this;
  }

  duration_type& operator=(millisecs_t d)
  {
    dur_ = d;
    return *this;
  }

  duration_type& operator=(seconds_t d)
  {
    dur_ = d;
    return *this;
  }

  duration_type& operator=(minutes_t d)
  {
    dur_ = d;
    return *this;
  }

  duration_type& operator=(hours_t d)
  {
    dur_ = d;
    return *this;
  }

#ifdef GCE_LUA
  std::string to_string()
  {
    std::string rt;
    rt += "<";
    switch (ty_)
    {
    case millisec:
      rt += boost::lexical_cast<std::string>(boost::chrono::duration_cast<millisecs_t>(dur_).count());
      break;
    case second:
      rt += boost::lexical_cast<std::string>(boost::chrono::duration_cast<seconds_t>(dur_).count());
      break;
    case minute:
      rt += boost::lexical_cast<std::string>(boost::chrono::duration_cast<minutes_t>(dur_).count());
      break;
    case hour:
      rt += boost::lexical_cast<std::string>(boost::chrono::duration_cast<hours_t>(dur_).count());
      break;
    default:
      rt += boost::lexical_cast<std::string>(dur_.count());
      break;
    }
    rt += ">";
    return rt;
  }

  int get_overloading_type() const
  {
    return (int)detail::overloading_duration;
  }

  GCE_LUA_SERIALIZE_FUNC
#endif

  duration_t dur_;
  type ty_;
};
}

#endif /// GCE_ACTOR_DURATION_HPP
