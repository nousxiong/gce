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
#include <gce/actor/actor_fwd.hpp>
#include <gce/actor/service_id.hpp>
#include <gce/actor/detail/listener.hpp>
#include <iostream>

namespace gce
{
namespace detail
{
struct actor_index
{
  actor_index()
    : id_(u32_nil)
    , svc_id_(u16_nil)
    , type_(actor_nil)
  {
  }

  operator bool() const
  {
    return type_ != actor_nil;
  }

  boost::uint32_t id_;
  boost::uint16_t svc_id_;
  detail::actor_type type_;
};
}

class actor_id
{
public:
  actor_id()
    : ctxid_(0)
    , timestamp_(0)
    , uintptr_(0)
    , svc_id_(0)
    , type_(0)
    , in_pool_(0)
    , sid_(0)
  {
  }

  actor_id(
    ctxid_t ctxid, timestamp_t timestamp,
    detail::listener* ptr, sid_t sid
    )
    : ctxid_(ctxid)
    , timestamp_(timestamp)
    , uintptr_((boost::uint64_t)ptr)
    , svc_id_(0)
    , type_(0)
    , in_pool_(0)
    , sid_(sid)
  {
  }

  actor_id(
    ctxid_t ctxid, timestamp_t timestamp,
    boost::uint32_t id, boost::uint16_t cac_id, 
    detail::actor_type type, sid_t sid
    )
    : ctxid_(ctxid)
    , timestamp_(timestamp)
    , uintptr_(id)
    , svc_id_(cac_id)
    , type_((byte_t)type)
    , in_pool_(1)
    , sid_(sid)
  {
  }

  ~actor_id()
  {
  }

public:
  operator bool() const
  {
    return timestamp_ != 0;
  }

  bool operator!() const
  {
    return timestamp_ == 0;
  }

  bool operator==(actor_id const& rhs) const
  {
    return
      ctxid_ == rhs.ctxid_ &&
      timestamp_ == rhs.timestamp_ &&
      uintptr_ == rhs.uintptr_ &&
      svc_id_ == rhs.svc_id_ &&
      type_ == rhs.type_ &&
      in_pool_ == rhs.in_pool_ &&
      sid_ == rhs.sid_;
  }

  bool operator!=(actor_id const& rhs) const
  {
    return !(*this == rhs);
  }

  bool operator<(actor_id const& rhs) const
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

    if (in_pool_ < rhs.in_pool_)
    {
      return true;
    }
    else if (in_pool_ > rhs.in_pool_)
    {
      return false;
    }

    if (in_pool())
    {
      if (svc_id_ < rhs.svc_id_)
      {
        return true;
      }
      else if (svc_id_ > rhs.svc_id_)
      {
        return false;
      }

      if (type_ < rhs.type_)
      {
        return true;
      }
      else if (type_ > rhs.type_)
      {
        return false;
      }
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

  detail::listener* get_actor_ptr(ctxid_t ctxid, timestamp_t timestamp) const
  {
    BOOST_ASSERT(!in_pool());
    detail::listener* ret = 0;
    if (
      ctxid == ctxid_ && timestamp == timestamp_ && uintptr_ != 0
      )
    {
      ret = (detail::listener*)uintptr_;
    }
    return ret;
  }

  detail::actor_index get_actor_index(ctxid_t ctxid, timestamp_t timestamp) const
  {
    detail::actor_index ret;
    BOOST_ASSERT(in_pool());
    if (ctxid == ctxid_ && timestamp == timestamp_)
    {
      ret.id_ = uintptr_;
      ret.svc_id_ = svc_id_;
      ret.type_ = (detail::actor_type)type_;
    }
    return ret;
  }

  bool in_pool() const
  {
    return in_pool_ != 0;
  }

  bool equals(actor_id const& rhs) const
  {
    return *this == rhs;
  }

  ctxid_t ctxid_;
  timestamp_t timestamp_;
  boost::uint64_t uintptr_;
  boost::uint16_t svc_id_;
  byte_t type_;
  byte_t in_pool_;
  sid_t sid_;

  /// internal use
  void set_svcid(svcid_t svc)
  {
    svc_ = svc;
  }

#ifdef GCE_LUA
  int get_overloading_type() const
  {
    return (int)detail::overloading_aid;
  }

  bool is_nil() const
  {
    return !(*this);
  }

  std::string to_string()
  {
    std::string rt;
    rt += "<";
    rt += boost::lexical_cast<std::string>(ctxid_);
    rt += ".";
    rt += boost::lexical_cast<std::string>(timestamp_);
    rt += ".";
    rt += boost::lexical_cast<std::string>(uintptr_);
    rt += ".";
    if (in_pool())
    {
      rt += boost::lexical_cast<std::string>(svc_id_);
      rt += ".";
      rt += boost::lexical_cast<std::string>((int)type_);
      rt += ".";
    }
    rt += boost::lexical_cast<std::string>(sid_);
    rt += ".";
    rt += svc_.to_string();
    rt += ">";

    return rt;
  }

  GCE_LUA_SERIALIZE_FUNC
#endif

  svcid_t svc_;
};

typedef actor_id aid_t;
typedef actor_id sktaid_t;

#ifdef GCE_LUA
aid_t lua_aid()
{
  return aid_t();
}
#endif

namespace detail
{
bool check_local(aid_t const& id, ctxid_t ctxid)
{
  return id.ctxid_ == ctxid;
}

bool check_local_valid(aid_t const& id, ctxid_t ctxid, timestamp_t timestamp)
{
  BOOST_ASSERT(id.ctxid_ == ctxid);
  return id.timestamp_ == timestamp;
}
}
}

template<typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(
  std::basic_ostream<CharT, TraitsT>& strm, gce::aid_t const& aid
  )
{
  if (aid.in_pool())
  {
    strm << "<" << aid.ctxid_ << "." << aid.timestamp_ <<
      "." << aid.uintptr_ << "." << aid.svc_id_ << "." << (int)aid.type_ << 
      "." << aid.sid_ << "." << aid.svc_ << ">";
  }
  else
  {
    strm << "<" << aid.ctxid_ << "." << aid.timestamp_ <<
      "." << aid.uintptr_ << "." << aid.sid_ << "." << aid.svc_ << ">";
  }
  return strm;
}

GCE_PACK(gce::aid_t, (ctxid_)(timestamp_)(uintptr_)(svc_id_)(type_)(in_pool_)(sid_)(svc_));

#endif /// GCE_ACTOR_ACTOR_ID_HPP
