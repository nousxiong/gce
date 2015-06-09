///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SPAWN_ACTOR_HPP
#define GCE_ACTOR_DETAIL_SPAWN_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/exception.hpp>
#include <gce/actor/detail/stackful_actor.hpp>
#include <gce/actor/detail/stackless_actor.hpp>
#include <gce/actor/detail/actor_function.hpp>
#include <gce/actor/detail/actor_ref.hpp>

namespace gce
{
namespace detail
{
template <typename Context>
inline aid_t make_stackful_actor(
  aid_t const& sire, typename Context::stackful_service_t& svc,
  actor_func<stackful, Context> const& f, size_t stack_size, link_type type
  )
{
  typedef Context context_t;
  stackful_actor<context_t>* a = svc.make_actor();
  a->init(f.f_);
  if (sire != aid_nil)
  {
    send(*a, sire, msg_new_actor, (uint16_t)type);
  }
  aid_t aid = a->get_aid();
  a->start(stack_size);
  return aid;
}

template <typename Context>
inline aid_t make_stackless_actor(
  aid_t const& sire, typename Context::stackless_service_t& svc,
  actor_func<stackless, Context> const& f, link_type type
  )
{
  typedef Context context_t;
  stackless_actor<context_t>* a = svc.make_actor();
  a->init(f.f_);
  if (sire != aid_nil)
  {
    send(*a, sire, msg_new_actor, (uint16_t)type);
  }
  aid_t aid = a->get_aid();
  a->start();
  return aid;
}

template <typename Service, typename ActorRef>
inline Service& select_service(ActorRef sire, bool sync_sire)
{
  typedef typename ActorRef::context_t context_t;
  context_t& ctx = sire.get_context();
  if (sync_sire)
  {
    return ctx.select_service<Service>(sire.get_service().get_index());
  }
  else
  {
    return ctx.select_service<Service>();
  }
}

template <typename ActorRef>
inline aid_t end_spawn(ActorRef sire, link_type type)
{
  pattern patt;
  patt.add_match(msg_new_actor);
  message msg;
  aid_t aid = sire.recv(msg, patt);
  GCE_VERIFY(aid != aid_nil)(msg).msg("gce::spawn_exception").except<spawn_exception>();

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

template <typename Context>
inline void handle_spawn(
  actor_ref<stackless, Context> self, aid_t aid, message msg, 
  link_type type, boost::function<void (actor_ref<stackless, Context>, aid_t)> const& hdr
  )
{
  if (aid != aid_nil)
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

/// spawn stackful_actor using NONE stackless_actor
template <typename ActorRef, typename F>
inline aid_t spawn(
  stackful,
  ActorRef sire, F f, bool sync_sire,
  link_type type, size_t stack_size
  )
{
  typedef typename ActorRef::context_t context_t;
  typedef typename context_t::stackful_service_t service_t;

  service_t& svc = select_service<service_t>(sire, sync_sire);
  svc.get_strand().post(
    boost::bind(
      &make_stackful_actor<context_t>,
      sire.get_aid(), boost::ref(svc),
      make_actor_func<stackful, context_t>(f), stack_size, type
      )
    );
  return end_spawn(sire, type);
}

/// spawn stackless_actor using NONE stackless_actor
template <typename ActorRef, typename F>
inline aid_t spawn(
  stackless,
  ActorRef sire, F f, bool sync_sire,
  link_type type, size_t
  )
{
  typedef typename ActorRef::context_t context_t;
  typedef typename context_t::stackless_service_t service_t;

  service_t& svc = select_service<service_t>(sire, sync_sire);
  svc.get_strand().post(
    boost::bind(
      &make_stackless_actor<context_t>,
      sire.get_aid(), boost::ref(svc),
      make_actor_func<stackless, context_t>(f), type
      )
    );
  return end_spawn(sire, type);
}

#ifdef GCE_LUA
/// spawn lua_actor using NONE stackless_actor
template <typename ActorRef>
inline aid_t spawn(
  luaed,
  ActorRef sire, std::string const& script, 
  bool sync_sire, link_type type, size_t
  )
{
  typedef typename ActorRef::context_t context_t;
  typedef typename context_t::lua_service_t service_t;

  service_t& svc = select_service<service_t>(sire, sync_sire);
  svc.get_strand().post(
    boost::bind(
    &service_t::spawn_actor, &svc,
      script, sire.get_aid(), type
      )
    );
  return end_spawn(sire, type);
}
#endif

/// spawn stackless_actor using stackless_actor
template <typename Context, typename F, typename SpawnHandler>
inline void spawn(
  stackless,
  actor_ref<stackless, Context> sire, F f, SpawnHandler h,
  typename Context::stackless_service_t& svc,
  link_type type
  )
{
  typedef Context context_t;
  typedef boost::function<void (actor_ref<stackless, context_t>, aid_t)> spawn_handler_t;

  svc.get_strand().post(
    boost::bind(
      &make_stackless_actor<context_t>,
      sire.get_aid(), boost::ref(svc),
      make_actor_func<stackless, context_t>(f), type
      )
    );

  pattern patt;
  patt.add_match(detail::msg_new_actor);
  sire.recv(
    boost::bind(
      &handle_spawn<context_t>, _arg1, _arg2, _arg3,
      type, spawn_handler_t(h)
      ),
    patt
    );
}

#ifdef GCE_LUA
/// spawn lua_actor using stackless_actor
template <typename Context, typename SpawnHandler>
inline void spawn(
  luaed,
  actor_ref<stackless, Context> sire, SpawnHandler h, 
  std::string const& script, typename Context::lua_service_t& svc, 
  link_type type
  )
{
  typedef Context context_t;
  typedef typename context_t::lua_service_t service_t;
  typedef boost::function<void (actor_ref<stackless, context_t>, aid_t)> spawn_handler_t;

  svc.get_strand().post(
    boost::bind(
    &service_t::spawn_actor, &svc,
      script, sire.get_aid(), type
      )
    );

  pattern patt;
  patt.add_match(detail::msg_new_actor);
  sire.recv(
    boost::bind(
      &handle_spawn<context_t>, _arg1, _arg2, _arg3,
      type, spawn_handler_t(h)
      ),
    patt
    );
}
#endif

template <typename Context>
inline void handle_remote_spawn(
  actor_ref<stackless, Context> self, aid_t aid,
  message msg, link_type type,
  boost::chrono::system_clock::time_point begin_tp,
  sid_t sid, duration_t tmo, duration_t curr_tmo,
  boost::function<void (actor_ref<stackless, Context>, aid_t)> const& hdr
  )
{
  typedef Context context_t;

  uint16_t err = 0;
  sid_t ret_sid = sid_nil;
  if (msg.get_type() == match_nil)
  {
    /// timeout
    hdr(self, aid);
    return;
  }

  msg >> err >> ret_sid;
  do
  {
    if (err != 0 || (aid != aid_nil  && sid == ret_sid))
    {
      break;
    }

    if (tmo != infin)
    {
      duration_t pass_time = boost::chrono::system_clock::now() - begin_tp;
      curr_tmo -= pass_time;
    }

    begin_tp = boost::chrono::system_clock::now();
    pattern patt;
    patt.add_match(detail::msg_spawn_ret);
    patt.timeout_ = curr_tmo;
    self.recv(
      boost::bind(
        &handle_remote_spawn<context_t>, _arg1, _arg2, _arg3,
        type, begin_tp, sid, tmo, curr_tmo, hdr
        ),
      patt
      );
    return;
  }
  while (false);

  detail::spawn_error error = (detail::spawn_error)err;
  if (error != detail::spawn_ok)
  {
    aid = aid_t();
  }

  if (aid != aid_nil)
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

template <typename ActorRef>
inline aid_t spawn_remote(
  spawn_type spw,
  ActorRef sire, std::string const& func, match_t ctxid,
  link_type type, size_t stack_size, duration_t tmo
  )
{
  aid_t aid;
  sid_t sid = sire.spawn(spw, func, ctxid, stack_size);
  uint16_t err = 0;
  sid_t ret_sid = sid_nil;

  duration_t curr_tmo = tmo;
  typedef boost::chrono::system_clock clock_t;
  clock_t::time_point begin_tp;

  do
  {
    begin_tp = clock_t::now();
    aid = sire->recv(detail::msg_spawn_ret, err, ret_sid, curr_tmo);
    if (err != 0 || (aid != aid_nil && sid == ret_sid))
    {
      break;
    }

    if (tmo != infin)
    {
      duration_t pass_time = from_chrono(clock_t::now() - begin_tp);
      curr_tmo = curr_tmo - pass_time;
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
        GCE_VERIFY(false)(spw)(ctxid)
          .msg("gce::no_socket_exception").except<no_socket_exception>();
      }break;
    case detail::spawn_func_not_found:
      {
        GCE_VERIFY(false)(spw)(ctxid)
          .msg("gce::func_not_found_exception").except<func_not_found_exception>();
      }break;
    default:
      {
        GCE_VERIFY(false)(spw)(ctxid)
          .msg("gce::spawn_exception").except<spawn_exception>();
      }break;
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

/// spawn remote stackful_actor using NONE stackless_actor
template <typename ActorRef>
inline aid_t spawn_remote(
  stackful,
  ActorRef sire, std::string const& func, match_t ctxid,
  link_type type, size_t stack_size, duration_t tmo
  )
{
  return spawn_remote(spw_stackful, sire, func, ctxid, type, stack_size, tmo);
}

/// spawn remote stackless_actor using NONE stackless_actor
template <typename ActorRef>
inline aid_t spawn_remote(
  stackless,
  ActorRef sire, std::string const& func, match_t ctxid,
  link_type type, size_t stack_size, duration_t tmo
  )
{
  return spawn_remote(spw_stackless, sire, func, ctxid, type, stack_size, tmo);
}

#ifdef GCE_LUA
/// spawn remote lua_actor using NONE stackless_actor
template <typename ActorRef>
inline aid_t spawn_remote(
  luaed,
  ActorRef sire, std::string const& func, match_t ctxid,
  link_type type, size_t stack_size, duration_t tmo
  )
{
  return spawn_remote(spw_luaed, sire, func, ctxid, type, stack_size, tmo);
}
#endif

template <typename Context, typename SpawnHandler>
inline void spawn_remote(
  spawn_type spw,
  actor_ref<stackless, Context> sire, std::string const& func, SpawnHandler h,
  match_t ctxid = ctxid_nil,
  link_type type = no_link,
  size_t stack_size = default_stacksize(),
  duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  typedef Context context_t;
  typedef boost::function<void (actor_ref<stackless, context_t>, aid_t)> spawn_handler_t;

  aid_t aid;
  sid_t sid = sire.spawn(spw, func, ctxid, stack_size);

  duration_t curr_tmo = tmo;
  typedef boost::chrono::system_clock clock_t;
  clock_t::time_point begin_tp = clock_t::now();
  pattern patt;
  patt.add_match(detail::msg_spawn_ret);
  patt.timeout_ = curr_tmo;
  sire.recv(
    boost::bind(
      &handle_remote_spawn<context_t>, _arg1, _arg2, _arg3,
      type, begin_tp, sid, tmo, curr_tmo, spawn_handler_t(h)
      ),
    patt
    );
}

/// spawn remote stackful_actor using stackless_actor
template <typename Context, typename SpawnHandler>
inline void spawn_remote(
  stackful,
  actor_ref<stackless, Context>& sire, std::string const& func, SpawnHandler h,
  match_t ctxid, link_type type, size_t stack_size, duration_t tmo
  )
{
  spawn_remote(spw_stackful, sire, func, h, ctxid, type, stack_size, tmo);
}

/// spawn remote stackless_actor using stackless_actor
template <typename Context, typename SpawnHandler>
inline aid_t spawn_remote(
  stackless,
  actor_ref<stackless, Context> sire, std::string const& func, SpawnHandler h,
  match_t ctxid, link_type type, size_t stack_size, duration_t tmo
  )
{
  return spawn_remote(spw_stackless, sire, func, h, ctxid, type, stack_size, tmo);
}

#ifdef GCE_LUA
/// spawn remote lua_actor using stackless_actor
template <typename Context, typename SpawnHandler>
inline aid_t spawn_remote(
  luaed,
  actor_ref<stackless, Context> sire, std::string const& func, SpawnHandler h,
  match_t ctxid, link_type type, size_t stack_size, duration_t tmo
  )
{
  return spawn_remote(spw_luaed, sire, func, h, ctxid, type, stack_size, tmo);
}
#endif
} /// namespace detail
} /// namespace gce

#endif /// GCE_ACTOR_DETAIL_SPAWN_ACTOR_HPP
