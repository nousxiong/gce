///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_SERVICE_ID_HPP
#define GCE_ACTOR_SERVICE_ID_HPP

#include <gce/actor/config.hpp>

namespace gce
{
class service_id
{
public:
  service_id()
    : ctxid_(ctxid_nil)
    , name_(match_nil)
  {
  }

  explicit service_id(match_t name)
    : ctxid_(ctxid_nil)
    , name_(name)
  {
  }

  service_id(ctxid_t ctxid, match_t name)
    : ctxid_(ctxid)
    , name_(name)
  {
  }

  ~service_id()
  {
  }

public:
  inline operator bool() const
  {
    return name_ != match_nil;
  }

  inline bool operator!() const
  {
    return name_ == match_nil;
  }

  inline bool operator==(service_id const& rhs) const
  {
    return
      ctxid_ == rhs.ctxid_ &&
      name_ == rhs.name_;
  }

  inline bool operator!=(service_id const& rhs) const
  {
    return !(*this == rhs);
  }

  inline bool operator<(service_id const& rhs) const
  {
    if (ctxid_ < rhs.ctxid_)
    {
      return true;
    }
    else if (ctxid_ > rhs.ctxid_)
    {
      return false;
    }

    if (name_ < rhs.name_)
    {
      return true;
    }
    else if (name_ > rhs.name_)
    {
      return false;
    }

    return false;
  }

#ifdef GCE_LUA
  /// internal use
  inline int get_overloading_type() const
  {
    return (int)detail::overloading_1;
  }

  inline std::string to_string()
  {
    std::string rt;
    rt += "<";
    rt += boost::lexical_cast<std::string>(ctxid_);
    rt += ".";
    rt += boost::lexical_cast<std::string>(name_);
    rt += ">";
    return rt;
  }

  GCE_LUA_SERIALIZE_FUNC
#endif

  ctxid_t ctxid_;
  match_t name_;
};

typedef service_id svcid_t;

#ifdef GCE_LUA
inline svcid_t lua_svcid()
{
  return svcid_t();
}
#endif
}

template<typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(
  std::basic_ostream<CharT, TraitsT>& strm, gce::svcid_t const& svc
  )
{
  strm << "<" << svc.ctxid_ << "." << svc.name_ << ">";
  return strm;
}

GCE_PACK(gce::svcid_t, (ctxid_)(name_));

#endif /// GCE_ACTOR_SERVICE_ID_HPP

