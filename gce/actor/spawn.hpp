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
#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>

namespace gce
{
namespace detail
{
inline aid_t make_stackful_actor(
  aid_t sire, cache_pool* user,
  actor_func<stackful> const& f, std::size_t stack_size
  )
{
  context& ctx = user->get_context();
  coroutine_stackful_actor* a = user->make_stackful_actor();
  a->init(f.f_);
  if (sire)
  {
    send(*a, sire, msg_new_actor);
  }
  a->start(stack_size);
  return a->get_aid();
}

inline aid_t make_stackless_actor(
  aid_t sire, cache_pool* user,
  actor_func<stackless> const& f, std::size_t stack_size
  )
{
  context& ctx = user->get_context();
  coroutine_stackless_actor* a = user->make_stackless_actor();
  a->init(f.f_);
  if (sire)
  {
    send(*a, sire, msg_new_actor);
  }
  a->start();
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

typedef boost::function<void (actor<stackless>&, aid_t)> spawn_handler_t;
inline void handle_spawn(
  actor<stackless>& self, aid_t aid,
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

  hdr(self, aid);
}

/// spawn coroutine_stackful_actor using NONE coroutine_stackless_actor
template <typename Sire, typename F>
inline aid_t spawn(
  stackful,
  Sire& sire, F f, cache_pool* user,
  link_type type, std::size_t stack_size
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_stackful_actor,
      sire.get_aid(), user,
      make_actor_func<stackful>(f), stack_size
      )
    );
  return end_spawn(sire, type);
}

/// spawn coroutine_stackless_actor using NONE coroutine_stackless_actor
template <typename Sire, typename F>
inline aid_t spawn(
  stackless,
  Sire& sire, F f, cache_pool* user,
  link_type type, std::size_t stack_size
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_stackless_actor,
      sire.get_aid(), user,
      make_actor_func<stackless>(f), stack_size
      )
    );
  return end_spawn(sire, type);
}

/// spawn coroutine_stackful_actor using coroutine_stackless_actor
template <typename F, typename SpawnHandler>
inline void spawn(
  stackful,
  actor<stackless>& sire, F f, SpawnHandler h,
  cache_pool* user, link_type type, std::size_t stack_size
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_stackful_actor,
      sire.get_aid(), user,
      make_actor_func<stackful>(f), stack_size
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

/// spawn coroutine_stackless_actor using coroutine_stackless_actor
template <typename F, typename SpawnHandler>
inline void spawn(
  stackless,
  actor<stackless>& sire, F f, SpawnHandler h,
  cache_pool* user, link_type type, std::size_t stack_size
  )
{
  user->get_strand().post(
    boost::bind(
      &detail::make_stackless_actor,
      sire.get_aid(), user,
      make_actor_func<stackless>(f), stack_size
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
  actor<stackless>& self, aid_t aid,
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

  hdr(self, aid);
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

/// spawn remote coroutine_stackful_actor using NONE coroutine_stackless_actor
template <typename Sire>
inline aid_t spawn(
  stackful,
  Sire& sire, match_t func, match_t ctxid,
  link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  return spawn(spw_stacked, sire, func, ctxid, type, stack_size, tmo);
}

/// spawn remote coroutine_stackless_actor using NONE coroutine_stackless_actor
template <typename Sire>
inline aid_t spawn(
  stackless,
  Sire& sire, match_t func, match_t ctxid,
  link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  return spawn(spw_evented, sire, func, ctxid, type, stack_size, tmo);
}

template <typename SpawnHandler>
inline void spawn(
  spawn_type spw,
  actor<stackless>& sire, match_t func, SpawnHandler h,
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
      type, begin_tp, sid, tmo, curr_tmo, spawn_handler_t(h)
      ),
    mach
    );
}

/// spawn remote coroutine_stackful_actor using coroutine_stackless_actor
template <typename SpawnHandler>
inline void spawn(
  stackful,
  actor<stackless>& sire, match_t func, SpawnHandler h,
  match_t ctxid, link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  spawn(spw_stacked, sire, func, h, ctxid, type, stack_size, tmo);
}

/// spawn remote coroutine_stackless_actor using coroutine_stackless_actor
template <typename SpawnHandler>
inline aid_t spawn(
  stackless,
  actor<stackless>& sire, match_t func, SpawnHandler& h,
  match_t ctxid, link_type type, std::size_t stack_size, seconds_t tmo
  )
{
  return spawn(spw_evented, sire, func, h, ctxid, type, stack_size, tmo);
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
  return detail::spawn(stackful(), sire, f, user, type, stack_size);
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
/// Spawn a actor using given coroutine_stackful_actor
///------------------------------------------------------------------------------
template <typename F>
inline aid_t spawn(
  actor<stackful>& sire, F f,
  link_type type = no_link,
  bool sync_sire = false,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = detail::select_cache_pool(sire, sync_sire);
  return detail::spawn(stackful(), sire, f, user, type, stack_size);
}

template <typename Tag, typename F>
inline aid_t spawn(
  actor<stackful>& sire, F f,
  link_type type = no_link,
  bool sync_sire = false,
  std::size_t stack_size = default_stacksize()
  )
{
  detail::cache_pool* user = detail::select_cache_pool(sire, sync_sire);
  return detail::spawn(Tag(), sire, f, user, type, stack_size);
}
///------------------------------------------------------------------------------
/// spawn a actor using given coroutine_stackless_actor
///------------------------------------------------------------------------------
template <typename F>
inline void spawn(
  actor<stackless>& sire, F f, aid_t& aid,
  link_type type = no_link,
  bool sync_sire = false,
  std::size_t stack_size = default_stacksize()
  )
{
  coroutine_stackless_actor& a = sire.get_actor();
  detail::cache_pool* user = detail::select_cache_pool(sire, sync_sire);
  detail::spawn(
    stackless(), sire, f, 
    boost::bind(
      &coroutine_stackless_actor::spawn_handler, &a, 
      _1, _2, boost::ref(aid)
      ), 
    user, type, stack_size
    );
}
///------------------------------------------------------------------------------
/// Spawn a thread_mapped_actor
///------------------------------------------------------------------------------
inline actor<threaded> spawn(context& ctx)
{
  return actor<threaded>(ctx.make_thread_mapped_actor());
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
/// Spawn a actor on remote context using NONE coroutine_stackless_actor
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
  return detail::spawn(stackful(), sire, func, ctxid, type, stack_size, tmo);
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
/// Spawn a actor on remote context using coroutine_stackless_actor
///------------------------------------------------------------------------------
inline void spawn(
  actor<stackless>& sire, match_t func, aid_t& aid,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  std::size_t stack_size = default_stacksize(),
  seconds_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  coroutine_stackless_actor& a = sire.get_actor();
  detail::spawn(
    stackful(), sire, func, 
    boost::bind(
      &coroutine_stackless_actor::spawn_handler, &a, 
      _1, _2, boost::ref(aid)
      ), 
    ctxid, type, stack_size, tmo
    );
}

template <typename Tag, typename SpawnHandler>
inline void spawn(
  actor<stackless>& sire, match_t func, SpawnHandler h,
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
