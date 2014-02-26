///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_SPAWN_HPP
#define GCE_ACTOR_SPAWN_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/slice.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace gce
{
namespace detail
{
template <typename F>
inline aid_t make_actor(
  cache_pool* cac_pool, F f,
  std::size_t stack_size,
  aid_t link_tgt, bool sync_sire
  )
{
  context& ctx = cac_pool->get_context();
  cache_pool* user = cac_pool;
  if (!sync_sire)
  {
    user = ctx.select_cache_pool();
  }
  actor* a = cac_pool->get_actor();
  a->init(user, cac_pool, f, link_tgt);
  a->start(stack_size);
  return a->get_aid();
}
}

/// Spawn a actor using given mixin
template <typename F>
inline aid_t spawn(
  mixin_t sire, F f,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_actor(sire.select_cache_pool(), f, stack_size, link_tgt, false);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}

/// Spawn a actor using given actor
template <typename F>
inline aid_t spawn(
  self_t sire, F f,
  link_type type = no_link,
  bool sync_sire = false,
  std::size_t stack_size = default_stacksize()
  )
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_actor(sire.get_cache_pool(), f, stack_size, link_tgt, sync_sire);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}

/// Spawn a mixin
inline mixin_t spawn(context& ctx)
{
  return ctx.make_mixin();
}

/// Spawn a slice using given mixin
inline slice_t spawn(mixin_t sire, link_type type = no_link)
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  slice_t s(sire.get_slice());
  s->init(link_tgt);
  sire.add_link(detail::link_t(type, s->get_aid()));
  return s;
}

/// Make actor function
template <typename F, typename A>
inline actor_func_t make_func(F f, A a)
{
  return actor_func_t(f, a);
}
}

#endif /// GCE_ACTOR_SPAWN_HPP
