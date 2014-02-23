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
#include <gce/actor/thin.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

namespace gce
{
namespace detail
{
inline aid_t make_actor(
  cache_pool* cac_pool,
  actor_func_t const& f,
  aid_t link_tgt,
  bool sync_sire
  )
{
  context* ctx = cac_pool->get_context();
  cache_pool* user = cac_pool;
  if (!sync_sire)
  {
    user = ctx->select_cache_pool();
  }
  actor* a = cac_pool->get_actor();
  a->init(user, cac_pool, f, link_tgt);
  a->start();
  return a->get_aid();
}

inline aid_t make_thin(
  cache_pool* cac_pool,
  thin_func_t const& f,
  aid_t link_tgt,
  bool sync_sire
  )
{
  context* ctx = cac_pool->get_context();
  cache_pool* user = cac_pool;
  if (!sync_sire)
  {
    user = ctx->select_cache_pool();
  }
  thin* t = cac_pool->get_thin();
  t->init(user, cac_pool, f, link_tgt);
  t->start();
  return t->get_aid();
}
}

struct stackful {};
struct stackless {};

namespace detail
{
/// Spawn a stackful actor using given mixin
inline aid_t spawn(stackful, mixin_t sire, actor_func_t const& f, link_type type)
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_actor(sire.select_cache_pool(), f, link_tgt, false);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}

/// Spawn a stackless actor using given mixin
inline aid_t spawn(stackless, mixin_t sire, thin_func_t const& f, link_type type)
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_thin(sire.select_cache_pool(), f, link_tgt, false);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}
}

template <typename Tag, typename F>
inline aid_t spawn(mixin_t sire, F f, link_type type = no_link)
{
  return detail::spawn(Tag(), sire, f, type);
}

namespace detail
{
/// Spawn a stackful actor using given actor
inline aid_t spawn(stackful, self_t sire, actor_func_t const& f, link_type type, bool sync_sire)
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_actor(sire.get_cache_pool(), f, link_tgt, sync_sire);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}

/// Spawn a stackless actor using given actor
inline aid_t spawn(stackless, self_t sire, thin_func_t const& f, link_type type, bool sync_sire)
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_thin(sire.get_cache_pool(), f, link_tgt, sync_sire);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}
}

template <typename Tag, typename F>
inline aid_t spawn(self_t sire, F f, link_type type = no_link, bool sync_sire = false)
{
  return detail::spawn(Tag(), sire, f, type, sync_sire);
}

namespace detail
{
/// Spawn a stackful actor using given thin
inline aid_t spawn(stackful, thin_t sire, actor_func_t const& f, link_type type, bool sync_sire)
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_actor(sire.get_cache_pool(), f, link_tgt, sync_sire);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}

/// Spawn a stackless actor using given thin
inline aid_t spawn(stackless, thin_t sire, thin_func_t const& f, link_type type, bool sync_sire)
{
  aid_t link_tgt;
  if (type != no_link)
  {
    link_tgt = sire.get_aid();
  }
  aid_t ret = make_thin(sire.get_cache_pool(), f, link_tgt, sync_sire);
  sire.add_link(detail::link_t(type, ret));
  return ret;
}
}

template <typename Tag, typename F>
inline aid_t spawn(thin_t sire, F f, link_type type = no_link, bool sync_sire = false)
{
  return detail::spawn(Tag(), sire, f, type, sync_sire);
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
  slice_t s(sire.get_slice_pool()->get());
  s->init(link_tgt);
  sire.add_link(detail::link_t(type, s->get_aid()));
  return s;
}

/// Make actor/thin function
template <typename T, typename F, typename A>
inline boost::function<void (T)> make_func(F f, A a)
{
  return boost::function<void (T)>(f, a);
}
}

#endif /// GCE_ACTOR_SPAWN_HPP
