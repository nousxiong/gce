///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_PATTERN_HPP
#define GCE_ACTOR_PATTERN_HPP

#include <gce/actor/config.hpp>

namespace gce
{
struct pattern
{
  pattern()
    : timeout_(infin)
  {
  }

  explicit pattern(duration_t tmo)
    : timeout_(tmo)
  {
  }

  explicit pattern(match_list_t match_list, duration_t tmo = infin)
    : timeout_(tmo)
    , match_list_(match_list)
  {
  }

  explicit pattern(match_t type1, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
  }

  pattern(match_t type1, match_t type2, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
    match_list_.push_back(type2);
  }

  pattern(match_t type1, match_t type2, match_t type3, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
    match_list_.push_back(type2);
    match_list_.push_back(type3);
  }

  pattern(match_t type1, match_t type2, match_t type3, match_t type4, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
    match_list_.push_back(type2);
    match_list_.push_back(type3);
    match_list_.push_back(type4);
  }

  pattern(match_t type1, match_t type2, match_t type3, match_t type4, match_t type5, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
    match_list_.push_back(type2);
    match_list_.push_back(type3);
    match_list_.push_back(type4);
    match_list_.push_back(type5);
  }

  void clear()
  {
    timeout_ = infin;
    match_list_.clear();
  }

#ifdef GCE_LUA
  /// internal use
  inline int get_overloading_type() const
  {
    return (int)detail::overloading_0;
  }

  inline void set_timeout(duration_t tmo)
  {
    timeout_ = tmo;
  }

  inline void add_match(match_type type)
  {
    match_list_.push_back(type);
  }

  inline std::string to_string()
  {
    std::string rt;
    rt += "<";
    rt += boost::lexical_cast<std::string>(timeout_.count());
    rt += ".";
    rt += boost::lexical_cast<std::string>(match_list_.size());
    rt += ">";
    return rt;
  }
#endif

  duration_t timeout_;
  match_list_t match_list_;
};

#ifdef GCE_LUA
inline pattern lua_pattern()
{
  return pattern();
}
#endif
}

#endif /// GCE_ACTOR_PATTERN_HPP

