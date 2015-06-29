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
#include <gce/actor/detail/listener.hpp>

namespace gce
{
namespace detail
{
template <typename Tag, typename Context>
class actor_ref {};

template <typename Actor>
class actor_ref_base
{
public:
  actor_ref_base()
    : a_(0)
  {
  }

  explicit actor_ref_base(Actor& a)
    : a_(&a)
  {
  }

  actor_ref_base(actor_ref_base const& other)
    : a_(other.a_)
  {
  }

  virtual ~actor_ref_base()
  {
  }

  actor_ref_base& operator=(actor_ref_base const& rhs)
  {
    if (this != &rhs)
    {
      a_ = rhs.a_;
    }
    return *this;
  }

  listener* get_listener()
  {
    return a_;
  }

public:
  void send(aid_t const& recver, message const& m)
  {
    a_->send(recver, m);
  }

  void send(svcid_t const& recver, message const& m)
  {
    a_->send(recver, m);
  }

  template <typename Recver>
  void send(Recver recver, message const& m)
  {
    a_->send(recver, m);
  }

  void relay(aid_t const& des, message& m)
  {
    a_->relay(des, m);
  }

  void relay(svcid_t const& des, message& m)
  {
    a_->relay(des, m);
  }

  template <typename Recver>
  void relay(Recver des, message& m)
  {
    a_->relay(des, m);
  }

  resp_t request(aid_t const& recver, message const& m)
  {
    return a_->request(recver, m);
  }

  resp_t request(svcid_t const& recver, message const& m)
  {
    return a_->request(recver, m);
  }

  template <typename Recver>
  resp_t request(Recver recver, message const& m)
  {
    return a_->request(recver, m);
  }

  void reply(aid_t const& recver, message const& m)
  {
    a_->reply(recver, m);
  }

  void link(aid_t const& target)
  {
    a_->link(target);
  }

  void monitor(aid_t const& target)
  {
    a_->monitor(target);
  }

  aid_t get_aid() const
  {
    return a_->get_aid();
  }

protected:
  Actor* a_;
};

///------------------------------------------------------------------------------
/// threaded actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<threaded, Context>
  : public actor_ref_base<typename Context::threaded_actor_t>
{
public:
  typedef Context context_t;
  typedef threaded type;
  typedef actor_wrap<actor_ref<threaded, Context>, false> actor_wrap_t;

private:
  typedef typename context_t::threaded_actor_t threaded_actor_t;
  typedef typename context_t::threaded_service_t service_t;
  typedef typename context_t::nonblocked_actor_t nonblocked_actor_t;
  typedef actor_ref_base<threaded_actor_t> base_t;

public:
  actor_ref()
  {
    aw_.set_actor_ref(*this);
  }

  explicit actor_ref(threaded_actor_t& a)
    : base_t(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : base_t(other)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref& operator=(actor_ref const& rhs)
  {
    if (this != &rhs)
    {
      base_t::operator=(rhs);
      aw_.set_actor_ref(*this);
    }
    return *this;
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  actor_wrap_t aw_;

public:
  aid_t recv(message& msg, pattern const& patt = pattern())
  {
    return base_t::a_->recv(msg, patt);
  }

  aid_t respond(
    resp_t const& res, message& msg, 
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    return base_t::a_->respond(res, msg, tmo);
  }

  void sleep_for(duration_t dur)
  {
    base_t::a_->sleep_for(dur);
  }

public:
  /// internal use
  sid_t spawn(
    detail::spawn_type type, std::string const& func, 
    match_t ctxid, size_t stack_size
    )
  {
    return base_t::a_->spawn(type, func, ctxid, stack_size);
  }

  context_t& get_context()
  {
    return base_t::a_->get_context();
  }

  service_t& get_service()
  {
    return base_t::a_->get_service();
  }

  std::vector<nonblocked_actor_t*>& get_nonblocked_actor_list() 
  { 
    return base_t::a_->get_nonblocked_actor_list();
  }

  void add_nonblocked_actor(nonblocked_actor_t& a)
  {
    base_t::a_->add_nonblocked_actor(a);
  }
};

///------------------------------------------------------------------------------
/// stackful actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<stackful, Context>
  : public actor_ref_base<typename Context::stackful_actor_t>
{
public:
  typedef Context context_t;
  typedef stackful type;
  typedef actor_wrap<actor_ref<stackful, Context>, false> actor_wrap_t;

private:
  typedef typename context_t::stackful_actor_t stackful_actor_t;
  typedef typename context_t::stackful_service_t service_t;
  typedef actor_ref_base<stackful_actor_t> base_t;

public:
  actor_ref()
  {
    aw_.set_actor_ref(*this);
  }

  explicit actor_ref(stackful_actor_t& a)
    : base_t(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : base_t(other)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref& operator=(actor_ref const& rhs)
  {
    if (this != &rhs)
    {
      base_t::operator=(rhs);
      aw_.set_actor_ref(*this);
    }
    return *this;
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  actor_wrap_t aw_;

public:
  aid_t recv(message& msg, pattern const& patt = pattern())
  {
    return base_t::a_->recv(msg, patt);
  }

  aid_t respond(
    resp_t const& res, message& msg, 
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    return base_t::a_->respond(res, msg, tmo);
  }

  void sleep_for(duration_t dur)
  {
    base_t::a_->sleep_for(dur);
  }

  yield_t get_yield()
  {
    return base_t::a_->get_yield();
  }

public:
  /// internal use
  sid_t spawn(
    detail::spawn_type type, std::string const& func, 
    match_t ctxid, size_t stack_size
    )
  {
    return base_t::a_->spawn(type, func, ctxid, stack_size);
  }

  context_t& get_context()
  {
    return base_t::a_->get_context();
  }

  service_t& get_service()
  {
    return base_t::a_->get_service();
  }
};

///------------------------------------------------------------------------------
/// stackless actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<stackless, Context>
  : public actor_ref_base<typename Context::stackless_actor_t>
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
  typedef actor_ref_base<stackless_actor_t> base_t;

public:
  actor_ref()
  {
    aw_.set_actor_ref(*this);
  }

  explicit actor_ref(stackless_actor_t& a)
    : base_t(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : base_t(other)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref& operator=(actor_ref const& rhs)
  {
    if (this != &rhs)
    {
      base_t::operator=(rhs);
      aw_.set_actor_ref(*this);
    }
    return *this;
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  actor_wrap_t aw_;

public:
  void recv(aid_t& sender, message& msg, pattern const& patt = pattern())
  {
    base_t::a_->recv(sender, msg, patt);
  }

  aid_t recv(message& msg, match_list_t const& match_list = match_list_t())
  {
    return base_t::a_->recv(msg, match_list);
  }

  void respond(
    resp_t const& res, aid_t& sender, message& msg, 
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    base_t::a_->respond(res, sender, msg, tmo);
  }

  aid_t respond(resp_t const& res, message& msg)
  {
    return base_t::a_->respond(res, msg);
  }

  void sleep_for(duration_t dur)
  {
    base_t::a_->sleep_for(dur);
  }

public:
  /// internal use
  void recv(recv_handler_t const& h, pattern const& patt = pattern())
  {
    base_t::a_->recv(h, patt);
  }

  void respond(
    recv_handler_t const& h, resp_t const& res, 
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    base_t::a_->respond(h, res, tmo);
  }

  void sleep_for(wait_handler_t const& h, duration_t dur)
  {
    base_t::a_->sleep_for(h, dur);
  }

  sid_t spawn(
    detail::spawn_type type, std::string const& func, 
    match_t ctxid, size_t stack_size
    )
  {
    return base_t::a_->spawn(type, func, ctxid, stack_size);
  }

  stackless_actor_t& get_actor()
  {
    return *base_t::a_;
  }

  detail::coro_t& coro() { return base_t::a_->coro(); }
  void resume() { base_t::a_->run(); }
  void sync_resume() 
  {
    base_t::a_->get_strand().dispatch(boost::bind(&stackless_actor_t::run, base_t::a_));
  }

  context_t& get_context()
  {
    return base_t::a_->get_context();
  }

  service_t& get_service()
  {
    return base_t::a_->get_service();
  }
};

///------------------------------------------------------------------------------
/// nonblocked actor
///------------------------------------------------------------------------------
template <typename Context>
class actor_ref<nonblocked, Context>
  : public actor_ref_base<typename Context::nonblocked_actor_t>
{
public:
  typedef Context context_t;
  typedef nonblocked type;
  typedef actor_wrap<actor_ref<nonblocked, Context>, false> actor_wrap_t;

private:
  typedef typename context_t::nonblocked_actor_t nonblocked_actor_t;
  typedef typename context_t::nonblocked_service_t service_t;
  typedef actor_ref_base<nonblocked_actor_t> base_t;

public:
  actor_ref()
  {
    aw_.set_actor_ref(*this);
  }

  explicit actor_ref(nonblocked_actor_t& a)
    : base_t(a)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref(actor_ref const& other)
    : base_t(other)
  {
    aw_.set_actor_ref(*this);
  }

  actor_ref& operator=(actor_ref const& rhs)
  {
    if (this != &rhs)
    {
      base_t::operator=(rhs);
      aw_.set_actor_ref(*this);
    }
    return *this;
  }

  actor_wrap_t* operator->()
  {
    return &aw_;
  }

private:
  actor_wrap_t aw_;

public:
  aid_t recv(message& msg, match_list_t const& match_list = match_list_t(), recver_t const& recver = recver_t())
  {
    return base_t::a_->recv(msg, match_list, recver);
  }

  aid_t respond(resp_t const& res, message& msg)
  {
    return base_t::a_->respond(res, msg);
  }

public:
  /// internal use
  context_t& get_context()
  {
    return base_t::a_->get_context();
  }

  service_t& get_service()
  {
    return base_t::a_->get_service();
  }
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACTOR_REF_HPP
