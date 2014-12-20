///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_GUARD_HPP
#define GCE_ACTOR_GUARD_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/detail/recver.hpp>

namespace gce
{
struct guard
{
  guard()
  {
  }

  explicit guard(aid_t const& aid)
    : recver_(aid)
  {
  }

  explicit guard(svcid_t const& svc)
    : recver_(svc)
  {
  }

  detail::recver_t recver_;
};
}

#endif /// GCE_ACTOR_GUARD_HPP
