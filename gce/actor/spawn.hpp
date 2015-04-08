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
#include <gce/actor/context.hpp>
#include <gce/actor/detail/spawn_actor.hpp>
#include <gce/actor/to_match.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

namespace gce
{
///------------------------------------------------------------------------------
/// Spawn a actor using given threaded_actor
///------------------------------------------------------------------------------
template <typename F>
inline aid_t spawn(
  threaded_actor sire, F f,
  link_type type = no_link,
  size_t stack_size = default_stacksize()
  )
{
#ifdef GCE_LUA
  typedef typename boost::remove_const<
    typename boost::remove_reference<
      typename boost::remove_volatile<F>::type
      >::type
    >::type param_t;
  typedef typename boost::mpl::if_<
    typename boost::is_same<param_t, char const*>::type, luaed, 
    typename boost::mpl::if_<
      typename boost::is_same<param_t, std::string>::type, luaed, stackful
      >::type
    >::type select_t;
#else
  typedef stackful select_t;
#endif
  return detail::spawn(select_t(), sire, f, false, type, stack_size);
}

template <typename Tag, typename F>
inline aid_t spawn(
  threaded_actor sire, F f,
  link_type type = no_link,
  size_t stack_size = default_stacksize()
  )
{
  return detail::spawn(Tag(), sire, f, false, type, stack_size);
}

///------------------------------------------------------------------------------
/// Spawn a actor using given stackful_actor
///------------------------------------------------------------------------------
template <typename F>
inline aid_t spawn(
  stackful_actor sire, F f,
  link_type type = no_link,
  bool sync_sire = false,
  size_t stack_size = default_stacksize()
  )
{
#ifdef GCE_LUA
  typedef typename boost::remove_const<
    typename boost::remove_reference<
      typename boost::remove_volatile<F>::type
      >::type
    >::type param_t;
  typedef typename boost::mpl::if_<
    typename boost::is_same<param_t, char const*>::type, luaed, 
    typename boost::mpl::if_<
      typename boost::is_same<param_t, std::string>::type, luaed, stackful
      >::type
    >::type select_t;
#else
  typedef stackful select_t;
#endif
  return detail::spawn(select_t(), sire, f, sync_sire, type, stack_size);
}

template <typename Tag, typename F>
inline aid_t spawn(
  stackful_actor sire, F f,
  link_type type = no_link,
  bool sync_sire = false,
  size_t stack_size = default_stacksize()
  )
{
  return detail::spawn(Tag(), sire, f, sync_sire, type, stack_size);
}

///------------------------------------------------------------------------------
/// spawn a actor using given stackless_actor
///------------------------------------------------------------------------------
namespace detail
{
template <typename F>
inline void spawn(
  stackless, 
  gce::stackless_actor sire, F f, aid_t& aid,
  link_type type, bool sync_sire
  )
{
  typedef typename context::stackless_actor_t stackless_actor_t;
  typedef typename context::stackless_service_t service_t;

  stackless_actor_t& a = sire.get_actor();
  service_t& svc = select_service<service_t>(sire, sync_sire);
  spawn(
    stackless(), sire, f, 
    boost::bind(
      &stackless_actor_t::spawn_handler, &a, 
      _arg1, _arg2, boost::ref(aid)
      ), 
    boost::ref(svc), type
    );
}

#ifdef GCE_LUA
inline void spawn(
  luaed, 
  gce::stackless_actor sire, std::string const& script, aid_t& aid,
  link_type type, bool sync_sire
  )
{
  typedef context::stackless_actor_t stackless_actor_t;
  typedef context::lua_service_t service_t;

  stackless_actor_t& a = sire.get_actor();
  service_t& svc = select_service<service_t>(sire, sync_sire);
  spawn(
    luaed(), sire, 
    boost::bind(
      &stackless_actor_t::spawn_handler, &a, 
      _arg1, _arg2, boost::ref(aid)
      ), 
    script, boost::ref(svc), type
    );
}
#endif
}

template <typename F>
inline void spawn(
  stackless_actor sire, F f, aid_t& aid,
  link_type type = no_link,
  bool sync_sire = false
  )
{
#ifdef GCE_LUA
  typedef typename boost::remove_const<
    typename boost::remove_reference<
      typename boost::remove_volatile<F>::type
      >::type
    >::type param_t;
  typedef typename boost::mpl::if_<
    typename boost::is_same<param_t, char const*>::type, luaed, 
    typename boost::mpl::if_<
      typename boost::is_same<param_t, std::string>::type, luaed, stackless
      >::type
    >::type select_t;
#else
  typedef stackless select_t;
#endif
  detail::spawn(select_t(), sire, f, aid, type, sync_sire);
}

///------------------------------------------------------------------------------
/// Spawn a threaded_actor
///------------------------------------------------------------------------------
inline threaded_actor spawn(context& ctx)
{
  return threaded_actor(ctx.make_threaded_actor());
}
///------------------------------------------------------------------------------
/// Spawn a nonblocked_actor using given threaded_actor
///------------------------------------------------------------------------------
inline nonblocked_actor spawn(threaded_actor a)
{
  context::nonblocked_actor_t& r = a.get_context().make_nonblocked_actor();
  a.add_nonblocked_actor(r);
  return nonblocked_actor(r);
}
///------------------------------------------------------------------------------
/// Spawn a actor on remote context using NONE stackless_actor
///------------------------------------------------------------------------------
template <typename ActorRef, typename Match>
inline aid_t spawn_remote(
  ActorRef sire, std::string const& func,
  Match ctxid,
  link_type type = no_link,
  size_t stack_size = default_stacksize(),
  duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  return detail::spawn_remote(stackful(), sire, func, to_match(ctxid), type, stack_size, tmo);
}

template <typename Tag, typename ActorRef, typename Match>
inline aid_t spawn_remote(
  ActorRef sire, std::string const& func,
  Match ctxid,
  link_type type = no_link,
  size_t stack_size = default_stacksize(),
  duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  return detail::spawn_remote(Tag(), sire, func, to_match(ctxid), type, stack_size, tmo);
}
///------------------------------------------------------------------------------
/// Spawn a actor on remote context using stackless_actor
///------------------------------------------------------------------------------
template <typename Match>
inline void spawn_remote(
  stackless_actor sire, std::string const& func, aid_t& aid,
  Match ctxid,
  link_type type = no_link,
  size_t stack_size = default_stacksize(),
  duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  typedef context::stackless_actor_t stackless_actor_t;

  stackless_actor_t& a = sire.get_actor();
  detail::spawn_remote(
    stackful(), sire, func, 
    boost::bind(
      &stackless_actor_t::spawn_handler, &a, 
      _arg1, _arg2, boost::ref(aid)
      ), 
    to_match(ctxid), type, stack_size, tmo
    );
}

template <typename Tag, typename SpawnHandler, typename Match>
inline void spawn_remote(
  stackless_actor sire, std::string const& func, SpawnHandler h,
  Match ctxid,
  link_type type = no_link,
  size_t stack_size = default_stacksize(),
  duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
  )
{
  detail::spawn_remote(Tag(), sire, func, h, to_match(ctxid), type, stack_size, tmo);
}
///------------------------------------------------------------------------------
}

#endif /// GCE_ACTOR_SPAWN_HPP
