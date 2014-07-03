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
#include <gce/actor/coroutine_stackful_actor.hpp>
#include <gce/actor/coroutine_stackless_actor.hpp>
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
class actor<stackful>
{
public:
  explicit actor(coroutine_stackful_actor& a)
    : a_(a)
  {
  }

private:
  coroutine_stackful_actor& a_;

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

  inline void chain(bool flag)
  {
    a_.chain(flag);
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

  inline void chain(bool flag)
  {
    a_.chain(flag);
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

  inline std::vector<nonblocking_actor*>& get_nonblocking_actor_list() 
  { 
    return a_.get_nonblocking_actor_list(); 
  }

  inline void add_nonblocking_actor(nonblocking_actor& a)
  {
    a_.add_nonblocking_actor(a);
  }
};

///------------------------------------------------------------------------------
/// stackless actor
///------------------------------------------------------------------------------
template <>
class actor<stackless>
{
public:
  explicit actor(coroutine_stackless_actor& a)
    : a_(a)
  {
  }

private:
  coroutine_stackless_actor& a_;

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

  inline void recv(aid_t& sender, message& msg, match const& mach = match())
  {
    a_.recv(sender, msg, mach);
  }

  inline aid_t recv(message& msg, match_list_t const& match_list = match_list_t())
  {
    return a_.recv(msg, match_list);
  }

  inline void recv(
    response_t res, aid_t& sender, message& msg, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    a_.recv(res, sender, msg, tmo);
  }

  inline aid_t recv(response_t res, message& msg)
  {
    return a_.recv(res, msg);
  }

  inline void wait(duration_t dur)
  {
    a_.wait(dur);
  }

  inline aid_t get_aid() const
  {
    return a_.get_aid();
  }

  inline void chain(bool flag)
  {
    a_.chain(flag);
  }

public:
  /// internal use
  inline void recv(recv_handler_t const& h, match const& mach = match())
  {
    a_.recv(h, mach);
  }

  inline void recv(
    recv_handler_t const& h, response_t res, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    a_.recv(h, res, tmo);
  }

  inline void wait(wait_handler_t const& h, duration_t dur)
  {
    a_.wait(h, dur);
  }

  inline sid_t spawn(
    detail::spawn_type type, match_t func, 
    match_t ctxid, std::size_t stack_size
    )
  {
    return a_.spawn(type, func, ctxid, stack_size);
  }

  inline coroutine_stackless_actor& get_actor()
  {
    return a_;
  }

  inline detail::coro_t& coro() { return a_.coro(); }
  inline void resume() { a_.run(); }

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
  BOOST_MPL_ASSERT((boost::mpl::or_<boost::is_same<Tag, stackful>, boost::is_same<Tag, stackless> >));

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
  remote_func(actor_func<stackful> const& f)
    : af_(f.f_)
  {
  }

  remote_func(actor_func<stackless> const& f)
    : ef_(f.f_)
  {
  }

  boost::function<void (actor<stackful>&)> af_;
  boost::function<void (actor<stackless>&)> ef_;
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

  inline void chain(bool flag)
  {
    a_.chain(flag);
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
