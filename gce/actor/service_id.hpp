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
#include <gce/actor/detail/to_match.hpp>

namespace gce
{
class service_id
{
public:
  service_id()
    : nil_(0)
    , ctxid_(0)
    , name_(0)
  {
  }

  template <typename Match>
  explicit service_id(Match name)
    : nil_(0)
    , ctxid_(0)
    , name_(detail::to_match(name))
  {
  }

  template <typename Ctxid, typename Match>
  service_id(Ctxid ctxid, Match name)
    : nil_(1)
    , ctxid_(detail::to_match(ctxid))
    , name_(detail::to_match(name))
  {
  }

  ~service_id()
  {
  }

public:
  operator bool() const
  {
    return nil_ != 0;
  }

  bool operator!() const
  {
    return nil_ == 0;
  }

  bool operator==(service_id const& rhs) const
  {
    return
      nil_ == rhs.nil_ && 
      ctxid_ == rhs.ctxid_ &&
      name_ == rhs.name_;
  }

  bool operator!=(service_id const& rhs) const
  {
    return !(*this == rhs);
  }

  bool operator<(service_id const& rhs) const
  {
    if (nil_ < rhs.nil_)
    {
      return true;
    }
    else if (ctxid_ > rhs.ctxid_)
    {
      return false;
    }

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

  std::string to_string() const
  {
    std::string rt;
    rt += "<";
    rt += boost::lexical_cast<std::string>(nil_);
    rt += ".";
    rt += boost::lexical_cast<std::string>(ctxid_);
    rt += ".";
    rt += boost::lexical_cast<std::string>(name_);
    rt += ">";
    return rt;
  }

#ifdef GCE_LUA
  /// internal use
  int get_overloading_type() const
  {
    return (int)detail::overloading_svcid;
  }

  GCE_LUA_SERIALIZE_FUNC
#endif

  boost::uint16_t nil_;
  ctxid_t ctxid_;
  match_t name_;
};

typedef service_id svcid_t;

#ifdef GCE_LUA
svcid_t lua_svcid()
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
  strm << "<" << svc.nil_ << "." << svc.ctxid_ << "." << svc.name_ << ">";
  return strm;
}

GCE_PACK(gce::svcid_t, (nil_)(ctxid_)(name_));

#endif /// GCE_ACTOR_SERVICE_ID_HPP

