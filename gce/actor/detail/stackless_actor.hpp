///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_STACKLESS_ACTOR_HPP
#define GCE_ACTOR_DETAIL_STACKLESS_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/actor_service.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/actor/detail/actor_ref.hpp>
#include <gce/actor/pattern.hpp>
#include <gce/actor/attributes.hpp>

namespace gce
{
namespace detail
{
template <typename Context>
class stackless_actor
  : public basic_actor<Context>
{
public:
  typedef Context context_t;

private:
  typedef basic_actor<context_t> base_t;
  typedef stackless_actor<context_t> self_t;
  typedef actor_service<self_t, true> service_t;

public:
  typedef actor_ref<stackless, context_t> self_ref_t;
  typedef boost::function<void (self_ref_t)> func_t;
  typedef boost::function<void (self_ref_t&, aid_t const&, message&)> recv_handler_t;

public:
  struct recv_binder
  {
    enum type
    {
      nil = 0,
      recv,
      spawn,
      bind,
      conn
    };

    recv_binder()
      : ty_(nil)
      , omsg_(0)
    {
    }

    recv_binder(self_t& self, aid_t& osender, message& omsg)
      : ty_(recv)
      , self_(&self)
      , osender_(&osender)
      , omsg_(&omsg)
    {
    }

    recv_binder(self_t& self, link_type lty, aid_t& osender)
      : ty_(spawn)
      , self_(&self)
      , lty_(lty)
      , osender_(&osender)
      , omsg_(0)
    {
    }

    explicit recv_binder(self_t& self)
      : ty_(bind)
      , self_(&self)
      , omsg_(0)
    {
    }

    recv_binder(self_t& self, service_t& sire, errcode_t& ec)
      : ty_(conn)
      , self_(&self)
      , omsg_(0)
      , sire_(&sire)
      , ec_(&ec)
    {
    }

    operator bool() const
    {
      return ty_ != nil;
    }

    void operator()(self_ref_t&, aid_t const& aid, message& m) const
    {
      GCE_ASSERT(ty_ != nil);
      switch (ty_)
      {
      case recv: self_->recv_handler(aid, m, *osender_, *omsg_); break;
      case spawn: self_->spawn_handler(aid, lty_, *osender_); break;
      case bind: self_->bind_handler(); break;
      case conn: self_->conn_handler(aid, m, *sire_, *ec_); break;
      default: GCE_ASSERT(false)(ty_); break;
      }
    }
    
    void clear()
    {
      ty_ = nil;
      omsg_ = 0;
    }

    type ty_;
    self_t* self_;
    link_type lty_;
    aid_t* osender_;
    message* omsg_;
    service_t* sire_;
    errcode_t* ec_;
  };

  struct wait_binder
  {
    wait_binder()
      : self_(0)
    {
    }

    explicit wait_binder(self_t& self)
      : self_(&self)
    {
    }

    operator bool() const
    {
      return self_ != 0;
    }

    void operator()() const
    {
      GCE_ASSERT(self_ != 0);
      self_->wait_handler();
    }

    void clear()
    {
      self_ = 0;
    }

    self_t* self_;
  };

public:
  stackless_actor(aid_t aid, service_t& svc)
    : base_t(svc.get_context(), svc, actor_stackless, aid)
    , aref_(*this)
    , svc_(svc)
    , tmr_(base_t::ctx_.get_io_service())
    , tmr_sid_(0)
    , lg_(base_t::ctx_.get_logger())
    , guard_(base_t::ctx_.get_io_service())
  {
    guard_.expires_from_now(to_chrono(infin));
  }

  ~stackless_actor()
  {
  }

public:
  void send(aid_t const& recver, message const& m)
  {
    base_t::pri_send(recver, m);
  }

  void send(svcid_t const& recver, message const& m)
  {
    base_t::pri_send_svc(recver, m);
  }

  template <typename Recver>
  void send(Recver recver, message const& m)
  {
    base_t::pri_send_svcs(recver, m);
  }

  void relay(aid_t const& des, message& m)
  {
    base_t::pri_relay(des, m);
  }

  void relay(svcid_t const& des, message& m)
  {
    base_t::pri_relay_svc(des, m);
  }

  template <typename Recver>
  void relay(Recver des, message& m)
  {
    base_t::pri_relay_svcs(des, m);
  }

  resp_t request(aid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request(res, recver, m);
    return res;
  }

  resp_t request(svcid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request_svc(res, recver, m);
    return res;
  }

  template <typename Recver>
  resp_t request(Recver recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid());
    base_t::pri_request_svcs(res, recver, m);
    return res;
  }

  void reply(aid_t const& recver, message const& m)
  {
    base_t::pri_reply(recver, m);
  }

  void link(aid_t const& target)
  {
    base_t::pri_link(target);
  }

  void monitor(aid_t const& target)
  {
    base_t::pri_monitor(target);
  }

  void link(svcid_t const& target)
  {
    base_t::pri_link_svc(target);
  }

  void monitor(svcid_t const& target)
  {
    base_t::pri_monitor_svc(target);
  }

  void recv(aid_t& sender, message& msg, pattern const& patt = pattern())
  {
    recv(recv_binder(*this, sender, msg), patt);
  }

  aid_t recv(message& msg, match_list_t const& match_list = match_list_t(), recver_t const& recver = recver_t())
  {
    aid_t sender;
    recv_t rcv;

    if (base_t::mb_.pop(rcv, msg, match_list, recver))
    {
      sender = end_recv(rcv, msg);
    }

    return sender;
  }

  void respond(
    resp_t res, aid_t& sender, message& msg, 
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    respond(recv_binder(*this, sender, msg), res, tmo);
  }

  aid_t respond(resp_t res, message& msg)
  {
    aid_t sender;

    if (base_t::mb_.pop(res, msg))
    {
      sender = end_recv(res);
    }

    return sender;
  }

  void sleep_for(duration_t dur)
  {
    sleep_for(wait_binder(*this), dur);
  }

public:
  /// internal use
  typedef gce::stackless type;

  static actor_type get_type()
  {
    return actor_stackless;
  }

  static size_t get_pool_reserve_size(attributes const& attr)
  {
    return attr.actor_pool_reserve_size_;
  }

  static size_t get_pool_max_size(attributes const& attr)
  {
    return attr.actor_pool_max_size_;
  }

  service_t& get_service()
  {
    return svc_;
  }

  void register_service(match_t name)
  {
    base_t::set_aid_svc(make_svcid(base_t::ctxid_, name));
  }

  void recv(recv_binder const& f, pattern const& patt = pattern())
  {
    recv(f, recv_b_, patt);
  }

  void recv(recv_handler_t const& f, pattern const& patt = pattern())
  {
    recv(f, recv_h_, patt);
  }

  void respond(
    recv_binder const& f, resp_t res,
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    respond(f, recv_b_, res, tmo);
  }

  void respond(
    recv_handler_t const& f, resp_t res,
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    respond(f, recv_h_, res, tmo);
  }

  void sleep_for(wait_binder const& f, duration_t dur)
  {
    wait_b_ = f;
    start_wait_timer(dur, f, wait_b_);
  }

  struct stop_binder
  {
    stop_binder(self_t& self, aid_t const& self_aid, exit_code_t exc, std::string const& errmsg)
      : self_(self)
      , self_aid_(self_aid)
      , exc_(exc)
      , errmsg_(errmsg)
    {
    }

    void operator()() const
    {
      self_.stop(self_aid_, exc_, errmsg_);
    }

    self_t& self_;
    aid_t self_aid_;
    exit_code_t exc_;
    std::string errmsg_;
  };

  void quit(
    exit_code_t exc = exit_normal,
    std::string const& errmsg = std::string()
    )
  {
    aid_t self_aid = base_t::get_aid();
    base_t::snd_.post(stop_binder(*this, self_aid, exc, errmsg));
  }

private:
  template <typename RecvHandler>
  void recv(RecvHandler const& f, RecvHandler& sf, pattern const& patt = pattern())
  {
    //aid_t sender;
    sender_ = aid_nil;
    recv_t rcv;
    //message msg;
    msg_.clear();
    message* pmsg = 0;

    if (!base_t::mb_.pop(rcv, pmsg, patt.match_list_, patt.recver_))
    {
      duration_t tmo = patt.timeout_;
      if (tmo > zero)
      {
        sf = f;
        curr_pattern_ = patt;
        is_resp_ = false;
        if (tmo < infin)
        {
          start_timer(tmo, f, sf);
        }
        return;
      }
    }
    else
    {
      sender_ = end_recv(rcv, *pmsg);
    }

    if (pmsg != 0)
    {
      msg_ = *pmsg;
      base_t::free_msg(pmsg);
    }

    base_t::snd_.post(
      recv_handler_binder<RecvHandler>(f, aref_, sender_, msg_)
      //boost::bind<void>(f, boost::ref(aref_), sender_, msg_)
      );
  }

  template <typename RecvHandler>
  void respond(
    RecvHandler const& f, RecvHandler& sf, resp_t res,
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    aid_t sender;
    msg_.clear();
    message* pmsg = 0;
    //message msg;

    if (!base_t::mb_.pop(res, pmsg))
    {
      if (tmo > zero)
      {
        sf = f;
        recving_res_ = res;
        is_resp_ = true;
        if (tmo < infin)
        {
          start_timer(tmo, f, sf);
        }
        return;
      }
    }
    else
    {
      sender = end_recv(res);
    }

    if (pmsg != 0)
    {
      msg_ = *pmsg;
      base_t::free_msg(pmsg);
    }

    base_t::snd_.post(
      recv_handler_binder<RecvHandler>(f, aref_, sender, msg_)
      //boost::bind<void>(f, boost::ref(aref_), sender, msg_)
      );
  }

  template <typename RecvHandler>
  struct recv_handler_binder
  {
    recv_handler_binder(
      RecvHandler const& h, 
      self_ref_t& aref, 
      aid_t const& sender, 
      message const& msg
      )
      : h_(h)
      , aref_(aref)
      , sender_(sender)
      , msg_(msg)
    {
    }

    void operator()()
    {
      h_(aref_, sender_, msg_);
    }

    RecvHandler h_;
    self_ref_t& aref_;
    aid_t sender_;
    message msg_;
  };

public:
  void init(func_t const& f)
  {
    f_ = f;
  }

  void start()
  {
    guard_.async_wait(guard());
    run();
  }

  void on_recv(pack& pk)
  {
    handle_recv(pk);
  }

  struct handle_recv_binder
  {
    handle_recv_binder(self_t& self, pack& pk)
      : self_(self)
      , pk_(pk)
    {
    }

    void operator()() const
    {
      self_.handle_recv(pk_);
    }

    self_t& self_;
    pack& pk_;
  };

  void on_addon_recv(pack& pk)
  {
    base_t::snd_.dispatch(handle_recv_binder(*this, pk));
  }

  sid_t spawn(
    spawn_type type, std::string const& func,
    match_t ctxid, size_t stack_size
    )
  {
    sid_t sid = base_t::new_request();
    base_t::pri_spawn(sid, type, func, ctxid, stack_size);
    return sid;
  }

  coro_t& coro()
  {
    return coro_;
  }

  void spawn_handler(aid_t const& sender, link_type lty, aid_t& osender)
  {
    if (sender != aid_nil)
    {
      if (lty == linked)
      {
        link(sender);
      }
      else if (lty == monitored)
      {
        monitor(sender);
      }
    }
    osender = sender;
    run();
  }

  void bind_handler()
  {
    run();
  }

  void conn_handler(aid_t const& skt, message& msg, service_t& sire, errcode_t& ec)
  {
    if (skt != aid_nil)
    {
      ctxid_pair_t ctxid_pr;
      msg >> ctxid_pr >> ec;
      sire.register_socket(ctxid_pr, skt);
    }

    run();
  }

  void run()
  {
    bool goon = false;
    try
    {
      f_(aref_);
      if (coro_.is_complete())
      {
        quit();
      }
      else
      {
        goon = true;
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      quit(exit_except, ex.what());
    }

    if (goon)
    {
      base_t::ctx_.on_tick(svc_.get_index());
    }
  }

private:
  void stop(aid_t self_aid, exit_code_t ec, std::string const& exit_msg)
  {
    errcode_t ignored_ec;
    guard_.cancel(ignored_ec);

    context_t& ctx = base_t::ctx_;
    size_t svc_id = svc_.get_index();
    base_t::send_exit(self_aid, ec, exit_msg);
    svc_.free_actor(this);
    ctx.on_tick(svc_id);
  }

  struct recv_timer_binder
  {
    recv_timer_binder(self_t& self, size_t tmr_sid, recv_binder const& rb, recv_binder& crb)
      : self_(self)
      , tmr_sid_(tmr_sid)
      , rb_(rb)
      , crb_(&crb)
    {
    }

    recv_timer_binder(self_t& self, size_t tmr_sid, recv_handler_t const& rh, recv_handler_t& crh)
      : self_(self)
      , tmr_sid_(tmr_sid)
      , rh_(rh)
      , crh_(&crh)
    {
    }

    void operator()(errcode_t const& ec) const
    {
      if (rb_)
      {
        self_.handle_recv_timeout(ec, tmr_sid_, rb_, *crb_);
      }
      else
      {
        self_.handle_recv_timeout(ec, tmr_sid_, rh_, *crh_);
      }
    }

    self_t& self_;
    size_t tmr_sid_;
    recv_binder rb_;
    recv_binder* crb_;
    recv_handler_t rh_;
    recv_handler_t* crh_;
  };

  template <typename RecvHandler>
  void start_timer(duration_t dur, RecvHandler const& hdr, RecvHandler& chdr)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(base_t::snd_.wrap(recv_timer_binder(*this, ++tmr_sid_, hdr, chdr)));
  }

  struct wait_timer_binder
  {
    wait_timer_binder(self_t& self, size_t tmr_sid, wait_binder const& wb, wait_binder& cwb)
      : self_(self)
      , tmr_sid_(tmr_sid)
      , wb_(wb)
      , cwb_(&cwb)
    {
    }

    void operator()(errcode_t const& ec) const
    {
      self_.handle_wait_timeout(ec, tmr_sid_, wb_, *cwb_);
    }

    self_t& self_;
    size_t tmr_sid_;
    wait_binder wb_;
    wait_binder* cwb_;
  };

  void start_wait_timer(duration_t dur, wait_binder const& hdr, wait_binder& chdr)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(base_t::snd_.wrap(wait_timer_binder(*this, ++tmr_sid_, hdr, chdr)));
  }

  template <typename RecvHandler>
  void handle_recv_timeout(
    errcode_t const& ec, size_t tmr_sid, RecvHandler const& hdr, RecvHandler& chdr
    )
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      GCE_ASSERT(chdr);
      chdr.clear();
      curr_pattern_.clear();
      message msg_nil;
      hdr(aref_, aid_nil, msg_nil);
    }
  }

  void handle_wait_timeout(
    errcode_t const& ec, size_t tmr_sid, wait_binder const& hdr, wait_binder& chdr
    )
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      GCE_ASSERT(chdr);
      chdr.clear();
      hdr();
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

    bool is_binder = recv_b_ ? true : false;
    if (is_binder || recv_h_)
    {
      //message* msg = 0;
      match_t ty = msg->get_type();
      msg = 0;

      if (!is_resp_ && !is_response)
      {
        recv_t rcv;
        //msg = is_binder && recv_b_.omsg_ != 0 ? recv_b_.omsg_ : &msg_;
        if (!base_t::mb_.pop(rcv, msg, curr_pattern_.match_list_, curr_pattern_.recver_))
        {
          return;
        }
        sender_ = end_recv(rcv, *msg);
      }

      if (is_resp_ && (is_response || ty == exit))
      {
        GCE_ASSERT(recving_res_.valid());
        //msg = is_binder && recv_b_.omsg_ != 0 ? recv_b_.omsg_ : &msg_;
        if (!base_t::mb_.pop(recving_res_, msg))
        {
          return;
        }
        sender_ = end_recv(recving_res_);
        recving_res_ = resp_t();
      }

      if (msg != 0)
      {
        if (is_binder)
        {
          tmp_b_ = recv_b_;
          recv_b_.clear();
        }
        else
        {
          tmp_h_ = recv_h_;
          recv_h_.clear();
        }

        curr_pattern_.clear();

        ++tmr_sid_;
        errcode_t ignored_ec;
        tmr_.cancel(ignored_ec);

        if (is_binder)
        {
          tmp_b_(aref_, sender_, *msg);
        }
        else
        {
          tmp_h_(aref_, sender_, *msg);
        }
        base_t::free_msg(msg);
      }
    }
  }

  aid_t end_recv(recv_t& rcv, message& msg)
  {
    aid_t sender;
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
    }
    return sender;
  }

  aid_t end_recv(resp_t& res)
  {
    return res.get_aid();
  }

  void recv_handler(aid_t const& sender, message& msg, aid_t& osender, message& omsg)
  {
    osender = sender;
    if (&msg != &omsg)
    {
      omsg = msg;
    }
    run();
  }

  void wait_handler()
  {
    run();
  }

  struct guard
  {
    void operator()(errcode_t const&) const {}
  };

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(func_t, f_)
  GCE_CACHE_ALIGNED_VAR(coro_t, coro_)

  /// coro local vars
  self_ref_t aref_;
  service_t& svc_;
  /*recv_handler_t recv_b_;
  recv_handler_t res_h_;*/
  bool is_resp_;
  recv_binder recv_b_;
  recv_binder tmp_b_;
  recv_handler_t recv_h_;
  recv_handler_t tmp_h_;
  wait_binder wait_b_;
  resp_t recving_res_;
  pattern curr_pattern_;
  aid_t sender_;
  message msg_;
  timer_t tmr_;
  size_t tmr_sid_;
  log::logger_t& lg_;

  timer_t guard_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_STACKLESS_ACTOR_HPP
