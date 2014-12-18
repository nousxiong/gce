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
#include <gce/actor/detail/recver.hpp>
#include <gce/actor/detail/to_match.hpp>

namespace gce
{
struct match
{
  template <typename Match>
  match(Match type, aid_t const& aid)
    : type_(detail::to_match(type))
    , recver_(aid)
  {
  }

  template <typename Match>
  match(Match type, svcid_t const& svc)
    : type_(detail::to_match(type))
    , recver_(svc)
  {
  }

  ~match()
  {
  }

  match_t type_;
  detail::recver_t recver_;
};
}

#endif /// GCE_ACTOR_MATCH_HPP
