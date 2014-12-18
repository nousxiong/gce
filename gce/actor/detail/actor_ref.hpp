///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_ACTOR_REF_HPP
#define GCE_ACTOR_DETAIL_ACTOR_REF_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/actor_wrap.hpp>

namespace gce
{
namespace detail
{
template <typename Tag, typename Context>
class actor_ref {};

///------------------------------------------------------------------------------
/// threaded actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<threaded, Context>
{
public:
  typedef Context context_t;
  typedef threaded type;
  typedef actor_wrap<actor_ref<threaded, Context>, false> actor_wrap_t;

private:
  typedef typename context_t::threaded_actor_t threaded_actor_t;
  typedef typename context_t::threaded_service_t service_t;
  typedef typename context_t::nonblocked_actor_t nonblocked_actor_t;

public:
  explicit actor_ref(threaded_actor_t& a)
    : a_(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : a_(other.a_)
  {
    aw_.set_actor_ref(*this);
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  threaded_actor_t& a_;
  actor_wrap_t aw_;

public:
  template <typename Recver>
  void send(Recver const& recver, message const& m)
  {
    a_.send(recver, m);
  }

  template <typename Recver>
  void relay(Recver const& des, message& m)
  {
    a_.relay(des, m);
  }

  template <typename Recver>
  resp_t request(Recver const& recver, message const& m)
  {
    return a_.request(recver, m);
  }

  void reply(aid_t const& recver, message const& m)
  {
    a_.reply(recver, m);
  }

  void link(aid_t const& target)
  {
    a_.link(target);
  }

  void monitor(aid_t const& target)
  {
    a_.monitor(target);
  }

  aid_t recv(message& msg, pattern const& patt = pattern())
  {
    return a_.recv(msg, patt);
  }

  aid_t respond(
    resp_t const& res, message& msg, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    return a_.respond(res, msg, tmo);
  }

  void sleep_for(duration_t dur)
  {
    a_.sleep_for(dur);
  }

  aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  sid_t spawn(
    detail::spawn_type type, std::string const& func, 
    match_t ctxid, std::size_t stack_size
    )
  {
    return a_.spawn(type, func, ctxid, stack_size);
  }

  context_t& get_context()
  {
    return a_.get_context();
  }

  service_t& get_service()
  {
    return a_.get_service();
  }

  std::vector<nonblocked_actor_t*>& get_nonblocked_actor_list() 
  { 
    return a_.get_nonblocked_actor_list();
  }

  void add_nonblocked_actor(nonblocked_actor_t& a)
  {
    a_.add_nonblocked_actor(a);
  }
};

///------------------------------------------------------------------------------
/// stackful actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<stackful, Context>
{
public:
  typedef Context context_t;
  typedef stackful type;
  typedef actor_wrap<actor_ref<stackful, Context>, false> actor_wrap_t;

private:
  typedef typename context_t::stackful_actor_t stackful_actor_t;
  typedef typename context_t::stackful_service_t service_t;

public:
  explicit actor_ref(stackful_actor_t& a)
    : a_(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : a_(other.a_)
  {
    aw_.set_actor_ref(*this);
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  stackful_actor_t& a_;
  actor_wrap_t aw_;

public:
  template <typename Recver>
  void send(Recver const& recver, message const& m)
  {
    a_.send(recver, m);
  }

  template <typename Recver>
  void relay(Recver const& des, message& m)
  {
    a_.relay(des, m);
  }

  template <typename Recver>
  resp_t request(Recver const& recver, message const& m)
  {
    return a_.request(recver, m);
  }

  void reply(aid_t const& recver, message const& m)
  {
    a_.reply(recver, m);
  }

  void link(aid_t const& target)
  {
    a_.link(target);
  }

  void monitor(aid_t const& target)
  {
    a_.monitor(target);
  }

  aid_t recv(message& msg, pattern const& patt = pattern())
  {
    return a_.recv(msg, patt);
  }

  aid_t respond(
    resp_t const& res, message& msg, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    return a_.respond(res, msg, tmo);
  }

  void sleep_for(duration_t dur)
  {
    a_.sleep_for(dur);
  }

  yield_t get_yield()
  {
    return a_.get_yield();
  }

  aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  sid_t spawn(
    detail::spawn_type type, std::string const& func, 
    match_t ctxid, std::size_t stack_size
    )
  {
    return a_.spawn(type, func, ctxid, stack_size);
  }

  context_t& get_context()
  {
    return a_.get_context();
  }

  service_t& get_service()
  {
    return a_.get_service();
  }
};

///------------------------------------------------------------------------------
/// stackless actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<stackless, Context>
{
public:
  typedef Context context_t;
  typedef stackless type;
  typedef actor_wrap<actor_ref<stackless, Context>, true> actor_wrap_t;

private:
  typedef typename context_t::stackless_actor_t stackless_actor_t;
  typedef typename context_t::stackless_service_t service_t;
  typedef typename stackless_actor_t::recv_handler_t recv_handler_t;
  typedef typename stackless_actor_t::wait_handler_t wait_handler_t;

public:
  explicit actor_ref(stackless_actor_t& a)
    : a_(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : a_(other.a_)
  {
    aw_.set_actor_ref(*this);
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  stackless_actor_t& a_;
  actor_wrap_t aw_;

public:
  template <typename Recver>
  void send(Recver const& recver, message const& m)
  {
    a_.send(recver, m);
  }

  template <typename Recver>
  void relay(Recver const& des, message& m)
  {
    a_.relay(des, m);
  }

  template <typename Recver>
  resp_t request(Recver const& recver, message const& m)
  {
    return a_.request(recver, m);
  }

  void reply(aid_t const& recver, message const& m)
  {
    a_.reply(recver, m);
  }

  void link(aid_t const& target)
  {
    a_.link(target);
  }

  void monitor(aid_t const& target)
  {
    a_.monitor(target);
  }

  void recv(aid_t& sender, message& msg, pattern const& patt = pattern())
  {
    a_.recv(sender, msg, patt);
  }

  aid_t recv(message& msg, match_list_t const& match_list = match_list_t())
  {
    return a_.recv(msg, match_list);
  }

  void respond(
    resp_t const& res, aid_t& sender, message& msg, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    a_.respond(res, sender, msg, tmo);
  }

  aid_t respond(resp_t const& res, message& msg)
  {
    return a_.respond(res, msg);
  }

  void sleep_for(duration_t dur)
  {
    a_.sleep_for(dur);
  }

  aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  void recv(recv_handler_t const& h, pattern const& patt = pattern())
  {
    a_.recv(h, patt);
  }

  void respond(
    recv_handler_t const& h, resp_t const& res, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    a_.respond(h, res, tmo);
  }

  void sleep_for(wait_handler_t const& h, duration_t dur)
  {
    a_.sleep_for(h, dur);
  }

  sid_t spawn(
    detail::spawn_type type, std::string const& func, 
    match_t ctxid, std::size_t stack_size
    )
  {
    return a_.spawn(type, func, ctxid, stack_size);
  }

  stackless_actor_t& get_actor()
  {
    return a_;
  }

  detail::coro_t& coro() { return a_.coro(); }
  void resume() { a_.run(); }
  void sync_resume() 
  {
    a_.get_strand().dispatch(boost::bind(&stackless_actor_t::run, &a_));
  }

  context_t& get_context()
  {
    return a_.get_context();
  }

  service_t& get_service()
  {
    return a_.get_service();
  }
};

///------------------------------------------------------------------------------
/// nonblocked actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<nonblocked, Context>
{
public:
  typedef Context context_t;
  typedef nonblocked type;
  typedef actor_wrap<actor_ref<nonblocked, Context>, false> actor_wrap_t;

private:
  typedef typename context_t::nonblocked_actor_t nonblocked_actor_t;
  typedef typename context_t::nonblocked_service_t service_t;

public:
  explicit actor_ref(nonblocked_actor_t& a)
    : a_(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : a_(other.a_)
  {
    aw_.set_actor_ref(*this);
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  nonblocked_actor_t& a_;
  actor_wrap_t aw_;

public:
  template <typename Recver>
  void send(Recver const& recver, message const& m)
  {
    a_.send(recver, m);
  }

  template <typename Recver>
  void relay(Recver const& des, message& m)
  {
    a_.relay(des, m);
  }

  template <typename Recver>
  resp_t request(Recver const& recver, message const& m)
  {
    return a_.request(recver, m);
  }

  void reply(aid_t const& recver, message const& m)
  {
    a_.reply(recver, m);
  }

  void link(aid_t const& target)
  {
    a_.link(target);
  }

  void monitor(aid_t const& target)
  {
    a_.monitor(target);
  }

  aid_t recv(message& msg, match_list_t const& match_list = match_list_t(), recver_t const& recver = recver_t())
  {
    return a_.recv(msg, match_list, recver);
  }

  aid_t respond(resp_t const& res, message& msg)
  {
    return a_.respond(res, msg);
  }

  aid_t get_aid() const
  {
    return a_.get_aid();
  }

public:
  /// internal use
  context_t& get_context()
  {
    return a_.get_context();
  }

  service_t& get_service()
  {
    return a_.get_service();
  }
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACTOR_REF_HPP
