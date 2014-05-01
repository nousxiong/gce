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
#include <gce/actor/recv.hpp>
#include <gce/actor/send.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/strand.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>

namespace gce
{
namespace detail
{
inline aid_t make_actor(
  thread* thr, aid_t sire, 
  actor_func_t const& f, std::size_t stack_size
  )
{
  actor* a = thr->get_cache_pool().get_actor();
  a->init(f);
  a->start(stack_size);
  if (sire)
  {
    a->send(sire, message(detail::msg_new_actor));
  }
  return a->get_aid();
}

template <typename Sire, typename F>
inline aid_t spawn(
  Sire& sire, thread* thr, pack* pk, F f,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  thr->post(
    pk,
    boost::bind(
      &make_actor,
      thr, sire.get_aid(), actor_func_t(f), stack_size
      )
    );

  aid_t aid = recv(sire, detail::msg_new_actor);
  if (!aid)
  {
    throw std::runtime_error("make actor failed!");
  }

  if (type == linked)
  {
    sire.link(aid);
  }
  else if (type == monitored)
  {
    sire.monitor(aid);
  }
  return aid;
}
}

/// Spawn a mixin
inline mixin_t spawn(context& ctx)
{
  return ctx.make_mixin();
}

/// Spawn a actor using given mixin
template <typename F>
inline aid_t spawn(
  strand<mixin> snd, F f,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  mixin_t sire = snd.get_sire();
  thread& thr = snd.get_thread();
  aid_t aid = detail::spawn(sire, &thr, sire.get_pack(), f, type, stack_size);
  sire.free_pack();
  return aid;
}

/// Spawn a actor using given actor
template <typename F>
inline aid_t spawn(
  strand<actor> snd, F f,
  link_type type = no_link,
  bool sync_sire = false,
  std::size_t stack_size = default_stacksize()
  )
{
  self_t sire = snd.get_sire();

  aid_t aid;
  if (sync_sire)
  {
    aid = make_actor(sire.get_thread(), aid_t(), f, stack_size);
    if (!aid)
    {
      throw std::runtime_error("make actor failed!");
    }
  }
  else
  {
    thread& thr = snd.get_thread();
    aid = detail::spawn(sire, &thr, thr.get_cache_pool().get_pack(), f, type, stack_size);
  }

  return aid;
}

/// Spawn a actor on remote context
template <typename Sire>
inline aid_t spawn(
  Sire& sire, match_t func,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize(),
  seconds_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  aid_t aid;
  sid_t sid = sire.spawn(func, ctxid, stack_size);
  boost::uint16_t err = 0;
  sid_t ret_sid = sid_nil;

  duration_t curr_tmo = tmo;
  typedef boost::chrono::system_clock clock_t;
  clock_t::time_point begin_tp;

  do
  {
    begin_tp = clock_t::now();
    aid = recv(sire, detail::msg_spawn_ret, err, ret_sid, curr_tmo);
    if (err != 0 || (aid && sid == ret_sid))
    {
      break;
    }

    if (tmo != infin)
    {
      duration_t pass_time = clock_t::now() - begin_tp;
      curr_tmo -= pass_time;
    }
  }
  while (true);

  detail::spawn_error error = (detail::spawn_error)err;
  if (error != detail::spawn_ok)
  {
    switch (error)
    {
    case detail::spawn_no_socket:
      {
        throw std::runtime_error("no router socket available");
      }break;
    case detail::spawn_func_not_found:
      {
        throw std::runtime_error("remote func not found");
      }break;
    default:
      BOOST_ASSERT(false);
      break;
    }
  }

  if (type == linked)
  {
    sire.link(aid);
  }
  else if (type == monitored)
  {
    sire.monitor(aid);
  }
  return aid;
}

/// Make actor function
template <typename F, typename A>
inline actor_func_t make_func(F f, A a)
{
  return actor_func_t(f, a);
}
}

#endif /// GCE_ACTOR_SPAWN_HPP
