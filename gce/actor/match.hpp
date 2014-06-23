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

namespace gce
{
struct match
{
  match()
    : timeout_(infin)
  {
  }

  explicit match(duration_t tmo)
    : timeout_(tmo)
  {
  }

  explicit match(match_list_t match_list, duration_t tmo = infin)
    : timeout_(tmo)
    , match_list_(match_list)
  {
  }

  explicit match(match_t type1, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
  }

  match(match_t type1, match_t type2, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
    match_list_.push_back(type2);
  }

  match(match_t type1, match_t type2, match_t type3, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
    match_list_.push_back(type2);
    match_list_.push_back(type3);
  }

  match(match_t type1, match_t type2, match_t type3, match_t type4, duration_t tmo = infin)
    : timeout_(tmo)
  {
    match_list_.push_back(type1);
    match_list_.push_back(type2);
    match_list_.push_back(type3);
    match_list_.push_back(type4);
  }

  match(match_t type1, match_t type2, match_t type3, match_t type4, match_t type5, duration_t tmo = infin)
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

  duration_t timeout_;
  match_list_t match_list_;
};
}

#endif /// GCE_ACTOR_MATCH_HPP

