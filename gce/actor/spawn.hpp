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
#include <gce/actor/event_based_actor.hpp>
#include <gce/actor/thread_mapped_actor.hpp>
#include <gce/actor/nonblocking_actor.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>

namespace gce
{
namespace detail
{
inline aid_t make_context_switching_actor(
  aid_t sire, cache_pool* user, 
  actor_func_t const& f, std::size_t stack_size
  )
{
  context& ctx = user->get_context();
  context_switching_actor* a = user->get_context_switching_actor();
  a->init(f);
  if (sire)
  {
    send(*a, sire, msg_new_actor);
  }
  a->start(stack_size);
  return a->get_aid();
}

inline aid_t make_event_based_actor(
  aid_t sire, cache_pool* user, 
  event_func_t const& f, std::size_t stack_size
  )
{
  context& ctx = user->get_context();
  event_based_actor* a = user->get_event_based_actor();
  a->init();
  if (sire)
  {
    send(*a, sire, msg_new_actor);
  }
  a->start(f);
  return a->get_aid();
}

template <typename Sire>
inline cache_pool* select_cache_pool(Sire& sire, bool sync_sire)
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
  return user;
}

template <typename Sire>
inline aid_t end_spawn(Sire& sire, link_type type)
{
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

typedef boost::function<void (actor<evented>&, aid_t)> spawn_handler_t;
inline void handle_spawn(
  actor<evented>& self, aid_t aid, 
  message msg, link_type type, spawn_handler_t const& hdr
  )
{
  if (aid)
  {
    if (type == linked)
    {
      self.link(aid);
    }
    else if (type == monitored)
    {
      self.monitor(aid);
    }
  }

  try
  {
    hdr(self, aid);
  }
  catch (std::exception& ex)
  {
    self.quit(exit_except, ex.what());
  }
}

/// spawn context_switching_actor using NONE event_based_actor
template <typename Sire, typename F>
inline aid_t spawn(
  stacked, 
  Sire& sire, F f, cache_pool* user, 
  link_type type, std::size_t stack_size
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_context_switching_actor,
      sire.get_aid(), user,
      actor_func_t(f), stack_size
      )
    );
  return end_spawn(sire, type);
}

/// spawn event_based_actor using NONE event_based_actor
template <typename Sire, typename F>
inline aid_t spawn(
  evented, 
  Sire& sire, F f, cache_pool* user, 
  link_type type, std::size_t
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_event_based_actor,
      sire.get_aid(), user,
      event_func_t(f), stack_size
      )
    );
  return end_spawn(sire, type);
}

/// spawn context_switching_actor using event_based_actor
template <typename F, typename SpawnHandler>
inline void spawn(
  stacked, 
  actor<evented>& sire, F f, SpawnHandler& h, 
  cache_pool* user, link_type type, std::size_t stack_size
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_context_switching_actor,
      sire.get_aid(), user,
      actor_func_t(f), stack_size
      )
    );

  match mach;
  mach.match_list_.push_back(detail::msg_new_actor);
  sire.recv(
    boost::bind(
      &handle_spawn, _1, _2, _3, 
      type, spawn_handler_t(h)
      ), 
    mach
    );
}

/// spawn event_based_actor using event_based_actor
template <typename F, typename SpawnHandler>
inline void spawn(
  evented, 
  actor<evented>& sire, F f, SpawnHandler& h, 
  cache_pool* user, link_type type, std::size_t
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_event_based_actor,
      sire.get_aid(), user,
      event_func_t(f), stack_size
      )
    );

  match mach;
  mach.match_list_.push_back(detail::msg_new_actor);
  sire.recv(
    boost::bind(
      &handle_spawn, _1, _2, _3, 
      type, spawn_handler_t(h)
      ), 
    mach
    );
}

inline void handle_remote_spawn(
  actor<evented>& self, aid_t aid, 
  message msg, link_type type, 
  boost::chrono::system_clock::time_point begin_tp,
  sid_t sid, seconds_t tmo, duration_t curr_tmo, 
  spawn_handler_t const& hdr
  )
{
  boost::uint16_t err = 0;
  sid_t ret_sid = sid_nil;
  msg >> err >> ret_sid;

  do
  {
    if (err != 0 || (aid && sid == ret_sid))
    {
      break;
    }

    if (tmo != infin)
    {
      duration_t pass_time = boost::chrono::system_clock::now() - begin_tp;
      curr_tmo -= pass_time;
    }

    begin_tp = boost::chrono::system_clock::now();
    match mach(curr_tmo);
    mach.match_list_.push_back(detail::msg_spawn_ret);
    self.recv(
      boost::bind(
        &handle_remote_spawn, _1, _2, _3, 
        type, begin_tp, sid, tmo, curr_tmo, hdr
        ), 
      mach
      );
    return;
  }
  while (false);

  detail::spawn_error error = (detail::spawn_error)err;
  if (error != detail::spawn_ok)
  {
    aid = aid_t();
  }

  if (aid)
  {
    if (type == linked)
    {
      self.link(aid);
    }
    else if (type == monitored)
    {
      self.monitor(aid);
    }
  }

  try
  {
    hdr(self, aid);
  }
  catch (std::exception& ex)
  {
    self.quit(exit_except, ex.what());
  }
}

template <typename Sire>
inline aid_t spawn(
  spawn_type spw,
  Sire& sire, match_t func, match_t ctxid, 
  link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  aid_t aid;
  sid_t sid = sire.spawn(spw, func, ctxid, stack_size);
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

/// spawn remote context_switching_actor using NONE event_based_actor
template <typename Sire>
inline aid_t spawn(
  stacked,
  Sire& sire, match_t func, match_t ctxid, 
  link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  return spawn(spw_stacked, sire, func, ctxid, type, stack_size, tmo);
}

/// spawn remote event_based_actor using NONE event_based_actor
template <typename Sire>
inline aid_t spawn(
  evented,
  Sire& sire, match_t func, match_t ctxid, 
  link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  return spawn(spw_evented, sire, func, ctxid, type, stack_size, tmo);
}

template <typename SpawnHandler>
inline void spawn(
  spawn_type spw,
  actor<evented>& sire, match_t func, SpawnHandler h,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize(),
  seconds_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  aid_t aid;
  sid_t sid = sire.spawn(spw, func, ctxid, stack_size);
  boost::uint16_t err = 0;
  sid_t ret_sid = sid_nil;

  duration_t curr_tmo = tmo;
  typedef boost::chrono::system_clock clock_t;
  clock_t::time_point begin_tp = clock_t::now();
  match mach(curr_tmo);
  mach.match_list_.push_back(detail::msg_spawn_ret);
  sire.recv(
    boost::bind(
      &handle_remote_spawn, _1, _2, _3, 
      type, begin_tp, sid, tmo, curr_tmo, hdr
      ), 
    mach
    );
}

/// spawn remote context_switching_actor using event_based_actor
template <typename SpawnHandler>
inline aid_t spawn(
  stacked,
  actor<evented>& sire, match_t func, SpawnHandler& h, 
  match_t ctxid, link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  return spawn(spw_stacked, sire, func, h, ctxid, type, stack_size, tmo);
}

/// spawn remote event_based_actor using NONE event_based_actor
template <typename SpawnHandler>
inline aid_t spawn(
  evented,
  actor<evented>& sire, match_t func, SpawnHandler& h, 
  match_t ctxid, link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  return spawn(spw_evented, sire, func, h, ctxid, type, stack_size, tmo);
}

inline actor<threaded> spawn(threaded, context& ctx)
{
  return actor<threaded>(ctx.make_thread_mapped_actor());
}

inline actor<nonblocked> spawn(nonblocked, context& ctx)
{
  nonblocking_actor& a = ctx.make_nonblocking_actor();
  ctx.add_nonblocking_actor(a);
  return actor<nonblocked>(a);
}
} /// namespace detail

///------------------------------------------------------------------------------
/// Spawn a actor using given thread_mapped_actor
///------------------------------------------------------------------------------
template <typename Sire, typename F>
inline aid_t spawn(
  Sire& sire, F f,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = sire.get_context()->select_cache_pool();
  return detail::spawn(stacked(), sire, f, user, type, stack_size);
}

template <typename Tag, typename Sire, typename F>
inline aid_t spawn(
  Sire& sire, F f,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = sire.get_context()->select_cache_pool();
  return detail::spawn(Tag(), sire, f, user, type, stack_size);
}
///------------------------------------------------------------------------------
/// Spawn a actor using given context_switching_actor
///------------------------------------------------------------------------------
template <typename F>
inline aid_t spawn(
  actor<stacked>& sire, F f,
  link_type type = no_link,
  bool sync_sire = false,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = detail::select_cache_pool(sire, sync_sire);
  return detail::spawn(stacked(), sire, f, user, type, stack_size);
}

template <typename Tag, typename F>
inline aid_t spawn(
  actor<stacked>& sire, F f,
  link_type type = no_link,
  bool sync_sire = false,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = detail::select_cache_pool(sire, sync_sire);
  return detail::spawn(Tag(), sire, f, user, type, stack_size);
}
///------------------------------------------------------------------------------
/// spawn a actor using given event_based_actor
///------------------------------------------------------------------------------
template <typename Sire, typename F, typename SpawnHandler>
inline aid_t spawn(
  actor<evented>& sire, F f, SpawnHandler h, 
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = sire.get_context()->select_cache_pool();
  return detail::spawn(stacked(), sire, f, h, user, type, stack_size);
}

template <typename Tag, typename Sire, typename F, typename SpawnHandler>
inline aid_t spawn(
  actor<evented>& sire, F f, SpawnHandler h, 
  link_type type = no_link,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = sire.get_context()->select_cache_pool();
  return detail::spawn(Tag(), sire, f, h, user, type, stack_size);
}
///------------------------------------------------------------------------------
/// Spawn a thread_mapped_actor
///------------------------------------------------------------------------------
inline actor<threaded> spawn(context& ctx)
{
  return detail::spawn(threaded(), ctx);
}

template <typename Tag>
inline actor<Tag> spawn(context& ctx)
{
  return detail::spawn(Tag(), ctx);
}
///------------------------------------------------------------------------------
/// Spawn a nonblocking_actor using given thread_mapped_actor
///------------------------------------------------------------------------------
inline actor<nonblocked> spawn(actor<threaded>& a)
{
  nonblocking_actor& r = a.get_context()->make_nonblocking_actor();
  a.add_nonblocking_actor(r);
  return actor<nonblocked>(r);
}
///------------------------------------------------------------------------------
/// Spawn a actor on remote context using NONE event_based_actor
///------------------------------------------------------------------------------
template <typename Sire>
inline aid_t spawn(
  Sire& sire, match_t func,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize(),
  seconds_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  return detail::spawn(stacked(), sire, func, ctxid, type, stack_size, tmo);
}

template <typename Tag, typename Sire>
inline aid_t spawn(
  Sire& sire, match_t func,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize(),
  seconds_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  return detail::spawn(Tag(), sire, func, ctxid, type, stack_size, tmo);
}
///------------------------------------------------------------------------------
/// Spawn a actor on remote context using event_based_actor
///------------------------------------------------------------------------------
template <typename SpawnHandler>
inline void spawn(
  actor<evented>& sire, match_t func, SpawnHandler h,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize(),
  seconds_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  detail::spawn(stacked(), sire, func, h, ctxid, type, stack_size, tmo);
}

template <typename Tag, typename SpawnHandler>
inline void spawn(
  actor<evented>& sire, match_t func, SpawnHandler h,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize(),
  seconds_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  detail::spawn(Tag(), sire, func, h, ctxid, type, stack_size, tmo);
}
///------------------------------------------------------------------------------
}

#endif /// GCE_ACTOR_SPAWN_HPP
