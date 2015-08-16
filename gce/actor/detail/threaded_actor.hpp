///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_THREADED_ACTOR_HPP
#define GCE_ACTOR_DETAIL_THREADED_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/actor_service.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/actor/pattern.hpp>
#include <boost/thread/future.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace gce
{
namespace detail
{
template <typename Context>
class threaded_actor
  : public basic_actor<Context>
{
public:
  typedef Context context_t;

private:
  typedef basic_actor<context_t> base_t;
  typedef threaded_actor<context_t> self_t;
  typedef actor_service<self_t, false> service_t;
  typedef typename context_t::nonblocked_actor_t nonblocked_actor_t;

public:
  threaded_actor(service_t& svc)
    : base_t(
        svc.get_context(), svc, actor_threaded,
        make_aid(
          svc.get_context().get_ctxid(),
          svc.get_context().get_timestamp(),
          this, sid_t(0)
          )
        )
    , svc_(svc)
    , recv_p_(0)
    , res_p_(0)
    , recv_msg_(0)
    , res_msg_(0)
    , tmr_(base_t::ctx_.get_io_service())
    , tmr_sid_(0)
  {
  }

  ~threaded_actor()
  {
  }

public:
  void send(aid_t const& recver, message const& m)
  {
    base_t::snd_.post(send_binder<aid_t>(*this, recver, m));
  }

  void send(svcid_t const& recver, message const& m)
  {
    base_t::snd_.post(send_binder<svcid_t>(*this, recver, m));
  }

  template <typename Recver>
  void send(Recver recver, message const& m)
  {
    base_t::snd_.post(send_binder<match_t>(*this, to_match(recver), m));
  }

  void relay(aid_t const& des, message& m)
  {
    base_t::snd_.post(relay_binder<aid_t>(*this, des, m));
  }

  void relay(svcid_t const& des, message& m)
  {
    base_t::snd_.post(relay_binder<svcid_t>(*this, des, m));
  }

  template <typename Recver>
  void relay(Recver des, message const& m)
  {
    base_t::snd_.post(relay_binder<match_t>(*this, to_match(des), m));
  }

  resp_t request(aid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::snd_.post(request_binder<aid_t>(*this, res, recver, m));
    return res;
  }

  resp_t request(svcid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::snd_.post(request_binder<svcid_t>(*this, res, recver, m));
    return res;
  }

  template <typename Recver>
  resp_t request(Recver recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid());
    base_t::snd_.post(request_binder<match_t>(*this, res, to_match(recver), m));
    return res;
  }

  void reply(aid_t const& recver, message const& m)
  {
    base_t::snd_.post(reply_binder(*this, recver, m));
  }

  void link(aid_t const& target)
  {
    base_t::snd_.post(link_binder<tag_link>(*this, target));
  }

  void monitor(aid_t const& target)
  {
    base_t::snd_.post(link_binder<tag_monitor>(*this, target));
  }

  void link(svcid_t const& target)
  {
    base_t::snd_.post(link_binder<tag_link>(*this, target));
  }

  void monitor(svcid_t const& target)
  {
    base_t::snd_.post(link_binder<tag_monitor>(*this, target));
  }

  aid_t recv(message& msg, pattern const& patt = pattern())
  {
    recv_promise_t p;
    recv_future_t f = p.get_future();

    base_t::snd_.post(try_recv_binder(*this, p, msg, patt));

    aid_t sender;
    recv_optional_t opt = f.get();
    if (opt)
    {
      recv_optional_t::reference_type rcv = boost::get(opt);
      if (aid_t* aid = boost::get<aid_t>(&rcv))
      {
        sender = *aid;
        msg.set_relay(*aid);
      }
      else if (request_t* req = boost::get<request_t>(&rcv))
      {
        sender = req->get_aid();
        msg.set_relay(*req);
      }
      else if (exit_t* ex = boost::get<exit_t>(&rcv))
      {
        sender = ex->get_aid();
      }/*
      msg = rcv.second;*/
    }
    return sender;
  }

  aid_t respond(
    resp_t res, message& msg, 
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    res_promise_t p;
    res_future_t f = p.get_future();

    base_t::snd_.post(try_response_binder(*this, p, msg, res, tmo));

    aid_t sender;
    res_optional_t opt = f.get();
    if (opt)
    {
      res = boost::get(opt);
      sender = res.get_aid();
      //msg = res_pr.second;
    }
    return sender;
  }

  void sleep_for(duration_t dur)
  {
    boost::this_thread::sleep_for(to_chrono(dur));
  }

public:
  /// internal use
  typedef gce::threaded type;

  static actor_type get_type()
  {
    return actor_threaded;
  }

  service_t& get_service()
  {
    return svc_;
  }

  void add_nonblocked_actor(nonblocked_actor_t& a)
  {
    nonblocked_actor_list_.push_back(&a);
  }

  std::vector<nonblocked_actor_t*>& get_nonblocked_actor_list()
  {
    return nonblocked_actor_list_;
  }

  void on_recv(pack& pk)
  {
    base_t::snd_.dispatch(handle_recv_binder(*this, pk));
  }

  void on_addon_recv(pack& pk)
  {
    base_t::snd_.dispatch(handle_recv_binder(*this, pk));
  }

  sid_t spawn(spawn_type type, std::string const& func, match_t ctxid, size_t stack_size)
  {
    sid_t sid = base_t::new_request();
    base_t::snd_.post(spawn_binder(*this, sid, type, func, ctxid, stack_size));
    return sid;
  }

private:
  typedef boost::optional<recv_t> recv_optional_t;
  typedef boost::optional<resp_t> res_optional_t;
  typedef boost::promise<recv_optional_t> recv_promise_t;
  typedef boost::promise<res_optional_t> res_promise_t;
  typedef boost::unique_future<recv_optional_t> recv_future_t;
  typedef boost::unique_future<res_optional_t> res_future_t;

  template <typename Recver>
  struct send_binder
  {
    send_binder(base_t& self, Recver const& recver, message const& m)
      : self_(self)
      , recver_(recver)
      , m_(m)
    {
    }

    void operator()() const
    {
      invoke(recver_);
    }

  private:
    void invoke(aid_t const& recver) const
    {
      self_.pri_send(recver, m_);
    }

    void invoke(svcid_t const& recver) const
    {
      self_.pri_send_svc(recver, m_);
    }

    void invoke(match_t const& recver) const
    {
      self_.pri_send_svcs(recver, m_);
    }

    base_t& self_;
    Recver const recver_;
    message const m_;
  };

  template <typename Recver>
  struct relay_binder
  {
    relay_binder(base_t& self, Recver const& des, message const& m)
      : self_(self)
      , des_(des)
      , m_(m)
    {
    }

    void operator()() const
    {
      invoke(des_);
    }

  private:
    void invoke(aid_t const& des) const
    {
      self_.pri_relay(des, m_);
    }

    void invoke(svcid_t const& des) const
    {
      self_.pri_relay_svc(des, m_);
    }

    void invoke(match_t const& des) const
    {
      self_.pri_relay_svcs(des, m_);
    }

    base_t& self_;
    Recver const des_;
    message const m_;
  };

  template <typename Recver>
  struct request_binder
  {
    request_binder(base_t& self, resp_t const& res, Recver const& recver, message const& m)
      : self_(self)
      , res_(res)
      , recver_(recver)
      , m_(m)
    {
    }

    void operator()() const
    {
      invoke(recver_);
    }

  private:
    void invoke(aid_t const& recver) const
    {
      self_.pri_request(res_, recver, m_);
    }

    void invoke(svcid_t const& recver) const
    {
      self_.pri_request_svc(res_, recver, m_);
    }

    void invoke(match_t const& recver) const
    {
      self_.pri_request_svcs(res_, recver, m_);
    }

    base_t& self_;
    resp_t const res_;
    Recver const recver_;
    message const m_;
  };

  struct reply_binder
  {
    reply_binder(base_t& self, aid_t const& recver, message const& m)
      : self_(self)
      , recver_(recver)
      , m_(m)
    {
    }

    void operator()() const
    {
      self_.pri_reply(recver_, m_);
    }

    base_t& self_;
    aid_t const recver_;
    message const m_;
  };

  struct tag_link {};
  struct tag_monitor {};

  template <typename Tag>
  struct link_binder
  {
    link_binder(base_t& self, aid_t const& target)
      : self_(self)
      , aid_(target)
      , svcid_(svcid_nil)
    {
    }

    link_binder(base_t& self, svcid_t const& target)
      : self_(self)
      , aid_(aid_nil)
      , svcid_(target)
    {
    }

    void operator()() const
    {
      invoke(Tag());
    }

  private:
    void invoke(tag_link) const
    {
      if (aid_ != aid_nil)
      {
        self_.pri_link(aid_);
      }
      else
      {
        self_.pri_link_svc(svcid_);
      }
    }

    void invoke(tag_monitor) const
    {
      if (aid_ != aid_nil)
      {
        self_.pri_monitor(aid_);
      }
      else
      {
        self_.pri_monitor_svc(svcid_);
      }
    }

    base_t& self_;
    aid_t const aid_;
    svcid_t const svcid_;
  };

  struct try_recv_binder
  {
    try_recv_binder(self_t& self, recv_promise_t& p, message& msg, pattern const& patt)
      : self_(self)
      , p_(p)
      , msg_(msg)
      , patt_(patt)
    {
    }

    void operator()() const
    {
      self_.try_recv(p_, msg_, patt_);
    }

    self_t& self_;
    recv_promise_t& p_;
    message& msg_;
    pattern const& patt_;
  };

  struct try_response_binder
  {
    try_response_binder(self_t& self, res_promise_t& p, message& msg, resp_t const& res, duration_t tmo)
      : self_(self)
      , p_(p)
      , msg_(msg)
      , res_(res)
      , tmo_(tmo)
    {
    }

    void operator()() const
    {
      self_.try_response(p_, msg_, res_, tmo_);
    }

    self_t& self_;
    res_promise_t& p_;
    message& msg_;
    resp_t const res_;
    duration_t tmo_;
  };

  struct handle_recv_binder
  {
    handle_recv_binder(self_t& self, pack const& pk)
      : self_(self)
      , pk_(pk)
    {
    }

    void operator()()
    {
      self_.handle_recv(pk_);
    }

    self_t& self_;
    pack pk_;
  };

  struct spawn_binder
  {
    spawn_binder(
      base_t& self, 
      sid_t const sid, 
      spawn_type const type, 
      std::string const& func, 
      match_t const& ctxid, 
      size_t const stack_size
      )
      : self_(self)
      , sid_(sid)
      , type_(type)
      , func_(func)
      , ctxid_(ctxid)
      , stack_size_(stack_size)
    {
    }

    void operator()() const
    {
      self_.pri_spawn(sid_, type_, func_, ctxid_, stack_size_);
    }

    base_t& self_;
    sid_t const sid_;
    spawn_type const type_;
    std::string const func_;
    match_t const ctxid_;
    size_t const stack_size_;
  };

  void try_recv(recv_promise_t& p, message& msg, pattern const& patt)
  {
    recv_t rcv;
    message* pmsg = 0;
    if (!base_t::mb_.pop(rcv, pmsg, patt.match_list_, patt.recver_))
    {
      duration_t tmo = patt.timeout_;
      if (tmo > zero)
      {
        recv_msg_ = &msg;
        recv_p_ = &p;
        curr_pattern_ = patt;
        if (tmo < infin)
        {
          start_timer(tmo, p);
        }
        return;
      }
    }

    if (pmsg != 0)
    {
      msg = *pmsg;
      base_t::free_msg(pmsg);
    }
    p.set_value(rcv);
  }

  void try_response(res_promise_t& p, message& msg, resp_t res, duration_t tmo)
  {
    message* pmsg = 0;
    if (!base_t::mb_.pop(res, pmsg))
    {
      if (tmo > zero)
      {
        res_msg_ = &msg;
        res_p_ = &p;
        recving_res_ = res;
        if (tmo < infin)
        {
          start_timer(tmo, p);
        }
        return;
      }
    }

    if (pmsg != 0)
    {
      msg = *pmsg;
      base_t::free_msg(pmsg);
    }
    p.set_value(res);
  }

  template <typename Promise>
  struct handle_timeout_binder
  {
    handle_timeout_binder(self_t& self, Promise& p, size_t const tmr_sid)
      : self_(self)
      , p_(p)
      , tmr_sid_(tmr_sid)
    {
    }

    void operator()(errcode_t const& ec) const
    {
      invoke(ec, p_);
    }

  private:
    void invoke(errcode_t const& ec, recv_promise_t& p) const
    {
      self_.handle_recv_timeout(ec, p, tmr_sid_);
    }

    void invoke(errcode_t const& ec, res_promise_t& p) const
    {
      self_.handle_res_timeout(ec, p, tmr_sid_);
    }

    self_t& self_;
    Promise& p_;
    size_t const tmr_sid_;
  };

  void start_timer(duration_t dur, recv_promise_t& p)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(
      base_t::snd_.wrap(
        handle_timeout_binder<recv_promise_t>(*this, p, ++tmr_sid_)
        )
      );
  }

  void start_timer(duration_t dur, res_promise_t& p)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(
      base_t::snd_.wrap(
        handle_timeout_binder<res_promise_t>(*this, p, ++tmr_sid_)
        )
      );
  }

  void handle_recv_timeout(errcode_t const& ec, recv_promise_t& p, size_t tmr_sid)
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      /// timed out
      GCE_ASSERT(&p == recv_p_);
      recv_p_ = 0;
      curr_pattern_.clear();
      //std::pair<recv_t, message> rcv;
      p.set_value(recv_t());
    }
  }

  void handle_res_timeout(errcode_t const& ec, res_promise_t& p, size_t tmr_sid)
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      /// timed out
      GCE_ASSERT(&p == res_p_);
      res_p_ = 0;
      recving_res_ = resp_t();
      //std::pair<resp_t, message> res_pr;
      p.set_value(recving_res_);
    }
  }

  void handle_recv(pack& pk)
  {
    bool is_response = false;
    message* msg = base_t::handle_pack(pk);

    if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
    {
      base_t::mb_.push(*aid, msg);
    }
    else if (request_t* req = boost::get<request_t>(&pk.tag_))
    {
      base_t::mb_.push(*req, msg);
    }
    else if (link_t* link = boost::get<link_t>(&pk.tag_))
    {
      base_t::add_link(link->get_aid(), pk.skt_);
      return;
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      base_t::mb_.push(*ex, msg);
      base_t::remove_link(ex->get_aid());
    }
    else if (resp_t* res = boost::get<resp_t>(&pk.tag_))
    {
      is_response = true;
      base_t::mb_.push(*res, msg);
    }

    match_t ty = msg->get_type();
    msg = 0;
    bool recving = recv_p_ != 0 && !is_response;
    bool resping = res_p_ != 0 && (is_response || ty == exit);

    if (recving || resping)
    {
      if (recving)
      {
        recv_t rcv;
        bool ret = base_t::mb_.pop(rcv, msg, curr_pattern_.match_list_, curr_pattern_.recver_);
        if (!ret)
        {
          return;
        }
        *recv_msg_ = *msg;
        base_t::free_msg(msg);
        recv_msg_= 0;
        curr_pattern_.clear();
        recv_promise_t* recv_p = recv_p_;
        recv_p_ = 0;
        recv_p->set_value(rcv);
      }

      if (resping)
      {
        GCE_ASSERT(recving_res_.valid());
        bool ret = base_t::mb_.pop(recving_res_, msg);
        if (!ret)
        {
          return;
        }
        *res_msg_ = *msg;
        base_t::free_msg(msg);
        res_msg_ = 0;
        res_promise_t* res_p = res_p_;
        res_p_ = 0;
        resp_t recving_res = recving_res_;
        recving_res_ = resp_t();
        res_p->set_value(recving_res);
      }

      ++tmr_sid_;
      errcode_t ec;
      tmr_.cancel(ec);
    }
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  /// local
  service_t& svc_;
  std::vector<nonblocked_actor_t*> nonblocked_actor_list_;
  recv_promise_t* recv_p_;
  res_promise_t* res_p_;
  message* recv_msg_;
  message* res_msg_;
  resp_t recving_res_;
  pattern curr_pattern_;
  timer_t tmr_;
  size_t tmr_sid_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_THREADED_ACTOR_HPP
