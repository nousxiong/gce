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
#include <gce/actor/context_switching_actor.hpp>
#include <gce/actor/event_based_actor.hpp>
#include <gce/actor/thread_mapped_actor.hpp>
#include <gce/actor/nonblocking_actor.hpp>
#include <boost/function.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/logical.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>

namespace gce
{
template <typename Tag>
class actor {};

///------------------------------------------------------------------------------
/// staced actor
///------------------------------------------------------------------------------
template <>
class actor<stacked>
{
public:
  friend class context_switching_actor;
  explicit actor(context_switching_actor& a)
    : a_(a)
  {
  }

private:
  context_switching_actor& a_;

public:
  inline void send(aid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void send(svcid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline void relay(svcid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline response_t request(aid_t recver, message const& m)
  {
    return a_.request(recver, m);
  }

  inline response_t request(svcid_t recver, message const& m)
  {
    return a_.request(recver, m);
  }

  inline void reply(aid_t recver, message const& m)
  {
    a_.reply(recver, m);
  }

  inline void link(aid_t target)
  {
    a_.link(target);
  }

  inline void monitor(aid_t target)
  {
    a_.monitor(target);
  }

  inline aid_t recv(message& msg, match const& mach = match())
  {
    return a_.recv(msg, mach);
  }

  inline aid_t recv(
    response_t res, message& msg, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    return a_.recv(res, msg, tmo);
  }

  inline void wait(duration_t dur)
  {
    a_.wait(dur);
  }

  inline yield_t get_yield()
  {
    return a_.get_yield();
  }

  inline aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  inline sid_t spawn(
    detail::spawn_type type, match_t func, 
    match_t ctxid, std::size_t stack_size
    )
  {
    return a_.spawn(type, func, ctxid, stack_size);
  }

  inline context* get_context()
  {
    return a_.get_context();
  }

  inline detail::cache_pool* get_cache_pool()
  {
    return a_.get_cache_pool();
  }
};

///------------------------------------------------------------------------------
/// threaded actor
///------------------------------------------------------------------------------
template <>
class actor<threaded>
{
public:
  friend class thread_mapped_actor;
  explicit actor(thread_mapped_actor& a)
    : a_(a)
  {
  }

private:
  thread_mapped_actor& a_;

public:
  inline void send(aid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void send(svcid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline void relay(svcid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline response_t request(aid_t recver, message const& m)
  {
    return a_.request(recver, m);
  }

  inline response_t request(svcid_t recver, message const& m)
  {
    return a_.request(recver, m);
  }

  inline void reply(aid_t recver, message const& m)
  {
    a_.reply(recver, m);
  }

  inline void link(aid_t target)
  {
    a_.link(target);
  }

  inline void monitor(aid_t target)
  {
    a_.monitor(target);
  }

  inline aid_t recv(message& msg, match const& mach = match())
  {
    return a_.recv(msg, mach);
  }

  inline aid_t recv(
    response_t res, message& msg, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    return a_.recv(res, msg, tmo);
  }

  inline void wait(duration_t dur)
  {
    a_.wait(dur);
  }

  inline aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  inline sid_t spawn(
    detail::spawn_type type, match_t func, 
    match_t ctxid, std::size_t stack_size
    )
  {
    return a_.spawn(type, func, ctxid, stack_size);
  }

  inline context* get_context()
  {
    return a_.get_context();
  }

  inline detail::cache_pool* get_cache_pool()
  {
    return a_.get_cache_pool();
  }

  inline void add_nonblocking_actor(nonblocking_actor& a)
  {
    a_.add_nonblocking_actor(a);
  }
};

///------------------------------------------------------------------------------
/// evented actor
///------------------------------------------------------------------------------
template <>
class actor<evented>
{
public:
  friend class event_based_actor;
  explicit actor(event_based_actor& a)
    : a_(a)
  {
  }

private:
  event_based_actor& a_;

public:
  inline void send(aid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void send(svcid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline void relay(svcid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline response_t request(aid_t recver, message const& m)
  {
    return a_.request(recver, m);
  }

  inline response_t request(svcid_t recver, message const& m)
  {
    return a_.request(recver, m);
  }

  inline void reply(aid_t recver, message const& m)
  {
    a_.reply(recver, m);
  }

  inline void link(aid_t target)
  {
    a_.link(target);
  }

  inline void monitor(aid_t target)
  {
    a_.monitor(target);
  }

  inline void recv(recv_handler_t const& hdr, match const& mach = match())
  {
    a_.recv(hdr, mach);
  }

  inline void recv(
    recv_handler_t const& hdr, response_t res, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    a_.recv(hdr, res, tmo);
  }

  inline void wait(wait_handler_t const& hdr, duration_t dur)
  {
    a_.wait(hdr, dur);
  }

  inline void quit(exit_code_t exc = exit_normal, std::string const& errmsg = std::string())
  {
    a_.quit(exc, errmsg);
  }

  inline aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  inline sid_t spawn(
    detail::spawn_type type, match_t func, 
    match_t ctxid, std::size_t stack_size
    )
  {
    return a_.spawn(type, func, ctxid, stack_size);
  }

  inline context* get_context()
  {
    return a_.get_context();
  }

  inline detail::cache_pool* get_cache_pool()
  {
    return a_.get_cache_pool();
  }
};

template <typename Tag>
struct actor_func
{
  BOOST_MPL_ASSERT((boost::mpl::or_<boost::is_same<Tag, stacked>, boost::is_same<Tag, evented> >));

  actor_func()
  {
  }

  template <typename F>
  actor_func(F f)
    : f_(f)
  {
  }

  template <typename F, typename A>
  actor_func(F f, A a)
    : f_(f, a)
  {
  }

  boost::function<void (actor<Tag>&)> f_;
};

template <typename Tag, typename F>
inline actor_func<Tag> make_actor_func(F f)
{
  return actor_func<Tag>(f);
}

template <typename Tag, typename F, typename A>
inline actor_func<Tag> make_actor_func(F f, A a)
{
  return actor_func<Tag>(f, a);
}

struct remote_func
{
  remote_func(actor_func<stacked> const& f)
    : af_(f.f_)
  {
  }

  remote_func(actor_func<evented> const& f)
    : ef_(f.f_)
  {
  }

  boost::function<void (actor<stacked>&)> af_;
  boost::function<void (actor<evented>&)> ef_;
};
typedef std::pair<match_t, remote_func> remote_func_t;
typedef std::vector<remote_func_t> remote_func_list_t;

///------------------------------------------------------------------------------
/// nonblocked actor
///------------------------------------------------------------------------------
template <>
class actor<nonblocked>
{
public:
  friend class nonblocking_actor;
  explicit actor(nonblocking_actor& a)
    : a_(a)
  {
  }

private:
  nonblocking_actor& a_;

public:
  inline void send(aid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void send(svcid_t recver, message const& m)
  {
    a_.send(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline void relay(svcid_t des, message& m)
  {
    a_.relay(des, m);
  }

  inline response_t request(aid_t recver, message const& m)
  {
    a_.request(recver, m);
  }

  inline response_t request(svcid_t recver, message const& m)
  {
    a_.request(recver, m);
  }

  inline void reply(aid_t recver, message const& m)
  {
    a_.reply(recver, m);
  }

  inline void link(aid_t target)
  {
    a_.link(target);
  }

  inline void monitor(aid_t target)
  {
    a_.monitor(target);
  }

  inline aid_t recv(message& msg, match_list_t const& match_list = match_list_t())
  {
    return a_.recv(msg, match_list);
  }

  inline aid_t recv(response_t res, message& msg)
  {
    return a_.recv(res, msg);
  }

  inline aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  inline context* get_context()
  {
    return a_.get_context();
  }

  inline detail::cache_pool* get_cache_pool()
  {
    return a_.get_cache_pool();
  }
};
}

#endif /// GCE_ACTOR_ACTOR_HPP
