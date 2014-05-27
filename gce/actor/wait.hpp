///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_WAIT_HPP
#define GCE_ACTOR_WAIT_HPP

#include <gce/actor/config.hpp>

namespace gce
{
template <typename Waiter>
inline void wait(Waiter& waiter, duration_t dur)
{
  return waiter.wait(dur);
}
}

#endif /// GCE_ACTOR_WAIT_HPP
