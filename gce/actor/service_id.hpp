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
#include <gce/actor/service_id.adl.h>
#include <gce/actor/match.adl.h>
#include <gce/actor/to_match.hpp>

namespace gce
{
inline std::string to_string(adl::service_id const& o)
{
  std::string str;
  str += "svcid<";
  str += boost::lexical_cast<intbuf_t>((int)o.valid_).cbegin();
  str += ".";
  str += to_string(o.ctxid_);
  str += ".";
  str += to_string(o.name_);
  str += ">";
  return str;
}

template <>
struct tostring<adl::service_id>
{
  static std::string convert(adl::service_id const& o)
  {
    return to_string(o);
  }
};

namespace adl
{
inline bool operator==(service_id const& lhs, service_id const& rhs)
{
  return
    lhs.valid_ == rhs.valid_ &&
    lhs.ctxid_ == rhs.ctxid_ &&
    lhs.name_ == rhs.name_
    ;
}

inline bool operator!=(service_id const& lhs, service_id const& rhs)
{
  return !(lhs == rhs);
}

inline bool operator<(service_id const& lhs, service_id const& rhs)
{
  if (lhs.valid_ < rhs.valid_)
  {
    return true;
  }
  else if (rhs.valid_ < lhs.valid_)
  {
    return false;
  }

  if (lhs.ctxid_ < rhs.ctxid_)
  {
    return true;
  }
  else if (rhs.ctxid_ < lhs.ctxid_)
  {
    return false;
  }

  if (lhs.name_ < rhs.name_)
  {
    return true;
  }
  else if (rhs.name_ < lhs.name_)
  {
    return false;
  }

  return false;
}
} /// namespace adl

/// service_id nil value
static adl::service_id const svcid_nil = adl::service_id();

typedef adl::service_id svcid_t;

template <typename Ctxid, typename Match>
inline svcid_t make_svcid(Ctxid ctxid, Match name)
{
  svcid_t svcid;
  svcid.ctxid_ = to_match(ctxid);
  svcid.name_ = to_match(name);
  svcid.valid_ = 1;
  return svcid;
}
} /// namespace gce

inline std::ostream& operator<<(std::ostream& strm, gce::svcid_t const& svcid)
{
  strm << gce::to_string(svcid);
  return strm;
}

GCE_PACK(gce::svcid_t, (v.valid_)(v.ctxid_)(v.name_));

#endif /// GCE_ACTOR_SERVICE_ID_HPP

