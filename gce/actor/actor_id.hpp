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
#include <iostream>

#define GCE_ACTOR_PTR_TYPE_NUM_LENGTH 10
#define GCE_ACTOR_TYPE_NUM_LENGTH 100
#define GCE_CAC_ID_NUM_LENGTH 10000

namespace gce
{
namespace detail
{
inline sid_t make_sid(sid_t src, bool flag)
{
  return src * GCE_ACTOR_PTR_TYPE_NUM_LENGTH + (sid_t)(flag ? 1 : 0);
}

inline boost::uint64_t make_uintptr(
  boost::uint32_t id, 
  boost::uint16_t cac_id, 
  detail::actor_type type
  )
{
  boost::uint64_t id_tmp = id;
  boost::uint64_t cac_id_tmp = cac_id;
  boost::uint64_t type_tmp = (boost::uint64_t)type;
  return 
    id_tmp * GCE_CAC_ID_NUM_LENGTH * GCE_ACTOR_TYPE_NUM_LENGTH + 
    cac_id_tmp * GCE_ACTOR_TYPE_NUM_LENGTH + 
    type_tmp;
}

struct actor_index
{
  actor_index()
    : id_(u32_nil)
    , cac_id_(u16_nil)
    , type_(actor_nil)
  {
  }

  inline operator bool() const
  {
    return type_ != actor_nil;
  }

  boost::uint32_t id_;
  boost::uint16_t cac_id_;
  detail::actor_type type_;
};
}

class basic_actor;
class actor_id
{
public:
  actor_id()
    : ctxid_(0)
    , timestamp_(0)
    , uintptr_(0)
    , cac_id_(0)
    , type_(0)
    , in_pool_(0)
    , sid_(0)
  {
  }

  actor_id(
    ctxid_t ctxid, timestamp_t timestamp,
    basic_actor* ptr, sid_t sid
    )
    : ctxid_(ctxid)
    , timestamp_(timestamp)
    , uintptr_((boost::uint64_t)ptr)
    , cac_id_(0)
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
    , cac_id_(cac_id)
    , type_((byte_t)type)
    , in_pool_(1)
    , sid_(sid)
  {
  }

  ~actor_id()
  {
  }

public:
  inline operator bool() const
  {
    return timestamp_ != 0;
  }

  inline bool operator!() const
  {
    return timestamp_ == 0;
  }

  inline bool operator==(actor_id const& rhs) const
  {
    return
      ctxid_ == rhs.ctxid_ &&
      timestamp_ == rhs.timestamp_ &&
      uintptr_ == rhs.uintptr_ &&
      cac_id_ == rhs.cac_id_ &&
      type_ == rhs.type_ &&
      in_pool_ == rhs.in_pool_ &&
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
      if (cac_id_ < rhs.cac_id_)
      {
        return true;
      }
      else if (cac_id_ > rhs.cac_id_)
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

  inline basic_actor* get_actor_ptr(ctxid_t ctxid, timestamp_t timestamp) const
  {
    BOOST_ASSERT(!in_pool());
    basic_actor* ret = 0;
    if (
      ctxid == ctxid_ && timestamp == timestamp_ && uintptr_ != 0
      )
    {
      ret = (basic_actor*)uintptr_;
    }
    return ret;
  }

  inline detail::actor_index get_actor_index(ctxid_t ctxid, timestamp_t timestamp) const
  {
    detail::actor_index ret;
    BOOST_ASSERT(in_pool());
    if (ctxid == ctxid_ && timestamp == timestamp_)
    {
      ret.id_ = uintptr_;
      ret.cac_id_ = cac_id_;
      ret.type_ = (detail::actor_type)type_;
    }
    return ret;
  }

  inline bool in_pool() const
  {
    return in_pool_ != 0;
  }

  ctxid_t ctxid_;
  timestamp_t timestamp_;
  boost::uint64_t uintptr_;
  boost::uint16_t cac_id_;
  byte_t type_;
  byte_t in_pool_;
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
  if (aid.in_pool())
  {
    strm << "<" << aid.ctxid_ << "." << aid.timestamp_ <<
      "." << aid.uintptr_ << "." << aid.cac_id_ << "." << (int)aid.type_ << 
      "." << aid.sid_ << "." << aid.svc_ << ">";
  }
  else
  {
    strm << "<" << aid.ctxid_ << "." << aid.timestamp_ <<
      "." << aid.uintptr_ << "." << aid.sid_ << "." << aid.svc_ << ">";
  }
  return strm;
}

GCE_PACK(gce::aid_t, (ctxid_)(timestamp_)(uintptr_)(cac_id_)(type_)(in_pool_)(sid_)(svc_));

#endif /// GCE_ACTOR_ACTOR_ID_HPP
