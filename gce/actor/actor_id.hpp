///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ACTOR_ID_HPP
#define GCE_ACTOR_ACTOR_ID_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/service_id.hpp>
#include <iostream>

namespace gce
{
class basic_actor;
class actor_id
{
public:
  actor_id()
    : ctxid_(ctxid_nil)
    , timestamp_(0)
    , uintptr_(0)
    , sid_(sid_nil)
  {
  }

  actor_id(
    ctxid_t ctxid, timestamp_t timestamp,
    basic_actor* ptr, sid_t sid
    )
    : ctxid_(ctxid)
    , timestamp_(timestamp)
    , uintptr_((boost::uint64_t)ptr)
    , sid_(sid)
  {
  }

  ~actor_id()
  {
  }

public:
  inline operator bool() const
  {
    return uintptr_ != 0;
  }

  inline bool operator!() const
  {
    return uintptr_ == 0;
  }

  inline bool operator==(actor_id const& rhs) const
  {
    return
      ctxid_ == rhs.ctxid_ &&
      timestamp_ == rhs.timestamp_ &&
      uintptr_ == rhs.uintptr_ &&
      sid_ == rhs.sid_;
  }

  inline bool operator!=(actor_id const& rhs) const
  {
    return !(*this == rhs);
  }

  inline bool operator<(actor_id const& rhs) const
  {
    if (ctxid_ < rhs.ctxid_)
    {
      return true;
    }
    else if (ctxid_ > rhs.ctxid_)
    {
      return false;
    }

    if (timestamp_ < rhs.timestamp_)
    {
      return true;
    }
    else if (timestamp_ > rhs.timestamp_)
    {
      return false;
    }

    if (uintptr_ < rhs.uintptr_)
    {
      return true;
    }
    else if (uintptr_ > rhs.uintptr_)
    {
      return false;
    }

    if (sid_ < rhs.sid_)
    {
      return true;
    }
    else if (sid_ > rhs.sid_)
    {
      return false;
    }

    return false;
  }

  inline basic_actor* get_actor_ptr(ctxid_t ctxid, timestamp_t timestamp) const
  {
    BOOST_ASSERT(ctxid == ctxid_);
    BOOST_ASSERT(timestamp == timestamp_);
    BOOST_ASSERT(uintptr_ != 0);
    return (basic_actor*)uintptr_;
  }

  ctxid_t ctxid_;
  timestamp_t timestamp_;
  boost::uint64_t uintptr_;
  sid_t sid_;

  /// internal use
  inline void set_svcid(svcid_t svc)
  {
    svc_ = svc;
  }

  svcid_t svc_;
};

typedef actor_id aid_t;
typedef actor_id sktaid_t;
}

template<typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(
  std::basic_ostream<CharT, TraitsT>& strm, gce::aid_t const& aid
  )
{
  strm << "<" << aid.ctxid_ << "." << aid.timestamp_ <<
    "." << aid.uintptr_ << "." << aid.sid_ << "." << aid.svc_ << ">";
  return strm;
}

GCE_PACK(gce::aid_t, (ctxid_)(timestamp_)(uintptr_)(sid_)(svc_));

#endif /// GCE_ACTOR_ACTOR_ID_HPP
