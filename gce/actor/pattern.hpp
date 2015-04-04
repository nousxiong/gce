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
#include <gce/actor/duration.hpp>
#include <gce/actor/detail/recver.hpp>
#include <gce/actor/to_match.hpp>
#include <boost/array.hpp>
#include <boost/variant/get.hpp>

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

  explicit pattern(match_list_t const& match_list, duration_t tmo = infin)
    : timeout_(tmo)
    , match_list_(match_list)
  {
  }

  template <typename Match1>
  explicit pattern(Match1 type1, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(to_match(type1));
  }

  template <typename Match1, typename Match2>
  pattern(Match1 type1, Match2 type2, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(to_match(type1));
    match_list_.push_back(to_match(type2));
  }

  template <typename Match1, typename Match2, typename Match3>
  pattern(Match1 type1, Match2 type2, Match3 type3, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(to_match(type1));
    match_list_.push_back(to_match(type2));
    match_list_.push_back(to_match(type3));
  }

  template <typename Match1, typename Match2, typename Match3, typename Match4>
  pattern(Match1 type1, Match2 type2, Match3 type3, Match4 type4, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(to_match(type1));
    match_list_.push_back(to_match(type2));
    match_list_.push_back(to_match(type3));
    match_list_.push_back(to_match(type4));
  }

  template <typename Match1, typename Match2, typename Match3, typename Match4, typename Match5>
  pattern(Match1 type1, Match2 type2, Match3 type3, Match4 type4, Match5 type5, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(to_match(type1));
    match_list_.push_back(to_match(type2));
    match_list_.push_back(to_match(type3));
    match_list_.push_back(to_match(type4));
    match_list_.push_back(to_match(type5));
  }

  void clear()
  {
    timeout_ = infin;
    match_list_.clear();
  }

  template <typename Match>
  void add_match(Match type)
  {
    match_list_.push_back(to_match(type));
  }

  duration_t timeout_;
  match_list_t match_list_;

  /// meta data, internal use
  detail::recver_t recver_;
};

inline std::string to_string(pattern const& o)
{
  std::string str;
  str += "patt<";
  str += gce::to_string(o.timeout_);
  str += ".";
  str += boost::lexical_cast<intbuf_t>(o.match_list_.size()).cbegin();
  str += ">";
  return str;
}

template <>
struct tostring<pattern>
{
  static std::string convert(pattern const& o)
  {
    return to_string(o);
  }
};
}

#endif /// GCE_ACTOR_PATTERN_HPP

