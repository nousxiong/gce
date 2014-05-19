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
#include <gce/actor/coroutine_stackfull_actor.hpp>
#include <gce/actor/thread_mapped_actor.hpp>
#include <gce/actor/slice.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>

namespace gce
{
namespace detail
{
typedef boost::promise<aid_t> actor_promise_t;
typedef boost::unique_future<aid_t> actor_future_t;

inline aid_t make_actor(
  aid_t sire, cache_pool* user, 
  actor_func_t const& f, std::size_t stack_size
  )
{
  context& ctx = user->get_context();
  actor* a = user->get_coroutine_stackfull_actor();
  a->init(f);
  if (sire)
  {
    send(*a, sire, msg_new_actor);
  }
  a->start(stack_size);
  return a->get_aid();
}

template <typename Sire, typename F>
inline aid_t spawn(
  Sire& sire, cache_pool* user, F& func, 
  link_type type, std::size_t stack_size
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_actor,
      sire.get_aid(), user,
      actor_func_t(func), stack_size
      )
    );

  match mach;
  mach.match_list_.push_back(detail::msg_new_actor);
  message msg;
  aid_t aid = sire.recv(msg, mach);
  if (!aid)
  {
    throw std::runtime_error("spawn actor failed!");
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

/// Spawn a actor using given thread_mapped_actor
template <typename Sire, typename F>
inline aid_t spawn(
  Sire& sire, F f,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = sire.get_context()->select_cache_pool();
  return detail::spawn(sire, user, f, type, stack_size);
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
  detail::cache_pool* user = 0;
  if (sync_sire)
  {
    user = sire.get_cache_pool();
  }
  else
  {
    user = sire.get_context()->select_cache_pool();
  }

  return detail::spawn(sire, user, f, type, stack_size);
}

/// Spawn a thread_mapped_actor
inline actor_t spawn(context& ctx)
{
  return ctx.make_mixin();
}

/// Spawn a slice
inline slice_t spawn(actor_t a)
{
  slice_t s = a.get_context()->make_slice();
  a.add_slice(s);
  return s;
}

inline slice_t spawn_slice(context& ctx)
{
  slice_t s = ctx.make_slice();
  ctx.add_slice(s);
  return s;
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
