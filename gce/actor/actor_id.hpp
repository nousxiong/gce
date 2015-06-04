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
#include <gce/actor/actor_id.adl.h>
#include <gce/actor/detail/listener.hpp>
#include <boost/array.hpp>
#include <iostream>

namespace gce
{
typedef uint64_t timestamp_t;

inline bool in_pool(adl::actor_id const& o)
{
  return o.in_pool_ != 0;
}

namespace adl
{
inline bool operator==(actor_id const& lhs, actor_id const& rhs)
{
  return
    lhs.ctxid_ == rhs.ctxid_ &&
    lhs.timestamp_ == rhs.timestamp_ &&
    lhs.uintptr_ == rhs.uintptr_ &&
    lhs.svc_id_ == rhs.svc_id_ &&
    lhs.type_ == rhs.type_ &&
    lhs.in_pool_ == rhs.in_pool_ &&
    lhs.sid_ == rhs.sid_
    ;
}

inline bool operator!=(actor_id const& lhs, actor_id const& rhs)
{
  return !(lhs == rhs);
}

inline bool operator<(actor_id const& lhs, actor_id const& rhs)
{
  if (lhs.ctxid_ < rhs.ctxid_)
  {
    return true;
  }
  else if (rhs.ctxid_ < lhs.ctxid_)
  {
    return false;
  }

  if (lhs.timestamp_ < rhs.timestamp_)
  {
    return true;
  }
  else if (rhs.timestamp_ < lhs.timestamp_)
  {
    return false;
  }

  if (lhs.in_pool_ < rhs.in_pool_)
  {
    return true;
  }
  else if (rhs.in_pool_ < lhs.in_pool_)
  {
    return false;
  }

  if (gce::in_pool(lhs))
  {
    if (lhs.svc_id_ < rhs.svc_id_)
    {
      return true;
    }
    else if (rhs.svc_id_ < lhs.svc_id_)
    {
      return false;
    }

    if (lhs.type_ < rhs.type_)
    {
      return true;
    }
    else if (rhs.type_ < lhs.type_)
    {
      return false;
    }
  }

  if (lhs.uintptr_ < rhs.uintptr_)
  {
    return true;
  }
  else if (rhs.uintptr_ < lhs.uintptr_)
  {
    return false;
  }

  if (lhs.sid_ < rhs.sid_)
  {
    return true;
  }
  else if (rhs.sid_ < lhs.sid_)
  {
    return false;
  }

  return false;
}
}

inline std::string to_string(adl::actor_id const& o)
{
  std::string str;
  str += "aid<";
  str += to_string(o.ctxid_);
  str += ".";
  str += boost::lexical_cast<intbuf_t>(o.timestamp_).cbegin();
  str += ".";
  str += boost::lexical_cast<intbuf_t>(o.uintptr_).cbegin();
  str += ".";
  if (in_pool(o))
  {
    str += boost::lexical_cast<intbuf_t>(o.svc_id_).cbegin();
    str += ".";
    str += boost::lexical_cast<intbuf_t>((int)o.type_).cbegin();
    str += ".";
  }
  str += boost::lexical_cast<intbuf_t>(o.sid_).cbegin();
  str += ".";
  str += to_string(o.svc_);
  str += ">";

  return str;
}

template <>
struct tostring<adl::actor_id>
{
  static std::string convert(adl::actor_id const& o)
  {
    return to_string(o);
  }
};

inline detail::listener* get_actor_ptr(adl::actor_id const& aid, ctxid_t ctxid, timestamp_t timestamp)
{
  GCE_ASSERT(!in_pool(aid))(aid);
  detail::listener* ret = 0;
  if (ctxid == aid.ctxid_ && timestamp == aid.timestamp_ && aid.uintptr_ != 0)
  {
    ret = (detail::listener*)aid.uintptr_;
  }
  return ret;
}

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

  uint32_t id_;
  uint16_t svc_id_;
  detail::actor_type type_;
};
}

inline detail::actor_index get_actor_index(adl::actor_id const& aid, ctxid_t ctxid, timestamp_t timestamp)
{
  detail::actor_index ret;
  GCE_ASSERT(in_pool(aid))(aid);
  if (ctxid == aid.ctxid_ && timestamp == aid.timestamp_)
  {
    ret.id_ = aid.uintptr_;
    ret.svc_id_ = aid.svc_id_;
    ret.type_ = (detail::actor_type)aid.type_;
  }
  return ret;
}

/// actor nil value
static adl::actor_id const aid_nil = adl::actor_id();

namespace detail
{
inline bool check_local(gce::adl::actor_id const& aid, gce::ctxid_t ctxid)
{
  if (aid == aid_nil)
  {
    return false;
  }
  return aid.ctxid_ == ctxid;
}

inline bool check_local_valid(gce::adl::actor_id const& aid, gce::ctxid_t ctxid, gce::timestamp_t timestamp)
{
  if (aid == aid_nil)
  {
    return false;
  }
  GCE_ASSERT(aid.ctxid_ == ctxid)(ctxid)(aid);
  return aid.timestamp_ == timestamp;
}
} /// namespace detail

typedef adl::actor_id aid_t;
typedef adl::actor_id sktaid_t;

inline gce::aid_t make_aid(ctxid_t ctxid, timestamp_t timestamp, detail::listener* ptr, sid_t sid)
{
  gce::aid_t aid;
  aid.ctxid_ = ctxid;
  aid.timestamp_ = timestamp;
  aid.uintptr_ = (uint64_t)ptr;
  aid.svc_id_ = 0;
  aid.type_ = 0;
  aid.in_pool_ = 0;
  aid.sid_ = sid;
  return aid;
}

inline gce::aid_t make_aid(ctxid_t ctxid, timestamp_t timestamp, uint32_t id, uint16_t cac_id, detail::actor_type type, sid_t sid)
{
  gce::aid_t aid;
  aid.ctxid_ = ctxid;
  aid.timestamp_ = timestamp;
  aid.uintptr_ = id;
  aid.svc_id_ = cac_id;
  aid.type_ = (byte_t)type;
  aid.in_pool_ = 1;
  aid.sid_ = sid;
  return aid;
}

} /// namespace gce

inline std::ostream& operator<<(std::ostream& strm, gce::aid_t const& aid)
{
  strm << gce::to_string(aid);
  return strm;
}

GCE_PACK(gce::aid_t, (v.ctxid_)(v.timestamp_)(v.uintptr_)(v.svc_id_)(v.type_)(v.in_pool_)(v.sid_)(v.svc_));

#endif /// GCE_ACTOR_ACTOR_ID_HPP
