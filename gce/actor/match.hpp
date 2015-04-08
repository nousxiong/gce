///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_MATCH_HPP
#define GCE_ACTOR_MATCH_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/match.adl.h>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>

namespace gce
{
namespace adl
{
inline bool operator==(match const& lhs, match const& rhs)
{
  return lhs.val_ == rhs.val_;
}

inline bool operator==(match const& lhs, int rhs)
{
  return lhs.val_ == rhs;
}

inline bool operator!=(match const& lhs, match const& rhs)
{
  return !(lhs == rhs);
}

inline bool operator!=(match const& lhs, int rhs)
{
  return !(lhs == rhs);
}

inline bool operator<(match const& lhs, match const& rhs)
{
  return lhs.val_ < rhs.val_;
}
} /// namespace adl

inline std::string to_string(adl::match const& o)
{
  std::string str;
  str += "match<";
  str += boost::lexical_cast<intbuf_t>(o.val_).cbegin();
  str += ">";
  return str;
}

template <>
struct tostring<adl::match>
{
  static std::string convert(adl::match const& o)
  {
    return to_string(o);
  }
};

typedef adl::match match_t;

inline match_t make_match(uint64_t val)
{
  match_t rt;
  rt.val_ = val;
  return rt;
}

/// match nil value
static match_t const match_nil = make_match(u64_nil);
typedef std::vector<match_t> match_list_t;
}

inline std::ostream& operator<<(std::ostream& strm, gce::match_t const& mat)
{
  strm << gce::to_string(mat);
  return strm;
}

GCE_PACK(gce::match_t, (v.val_));

#endif /// GCE_ACTOR_MATCH_HPP
