///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ACTOR_HPP
#define GCE_ACTOR_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/actor_ref.hpp>
#include <gce/actor/detail/actor_function.hpp>
#include <gce/actor/context.hpp>

namespace gce
{
typedef detail::actor_ref<threaded, context> threaded_actor;
typedef detail::actor_ref<stackful, context> stackful_actor;
typedef detail::actor_ref<stackless, context> stackless_actor;
typedef detail::actor_ref<nonblocked, context> nonblocked_actor;

template <typename Tag, typename F>
detail::actor_func<Tag, context> make_actor_function(F f)
{
  return detail::make_actor_func<Tag, context>(f);
}

template <typename Tag, typename F, typename A>
detail::actor_func<Tag, context> make_actor_function(F f, A a)
{
  return detail::make_actor_func<Tag, context>(f, a);
}
}

#endif /// GCE_ACTOR_ACTOR_HPP
