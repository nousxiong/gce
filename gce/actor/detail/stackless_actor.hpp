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
  typedef boost::function<void (self_ref_t, aid_t, message)> recv_handler_t;
  typedef boost::function<void (self_ref_t)> wait_handler_t;

public:
  stackless_actor(aid_t aid, service_t& svc)
    : base_t(svc.get_context(), svc, actor_stackless, aid)
    , svc_(svc)
    , tmr_(base_t::ctx_.get_io_service())
    , tmr_sid_(0)
    , lg_(base_t::ctx_.get_logger())
  {
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

  void recv(aid_t& sender, message& msg, pattern const& patt = pattern())
  {
    recv(
      boost::bind(
        &self_t::recv_handler, this, _arg1, _arg2, _arg3,
        boost::ref(sender), boost::ref(msg)
        ),
      patt
      );
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
    respond(
      boost::bind(
        &self_t::recv_handler, this, _arg1, _arg2, _arg3,
        boost::ref(sender), boost::ref(msg)
        ),
      res,
      tmo
      );
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
    sleep_for(
      boost::bind(
        &self_t::wait_handler, this, _arg1
        ),
      dur
      );
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

  service_t& get_service()
  {
    return svc_;
  }

  void recv(recv_handler_t const& f, pattern const& patt = pattern())
  {
    aid_t sender;
    recv_t rcv;
    message msg;

    if (!base_t::mb_.pop(rcv, msg, patt.match_list_, patt.recver_))
    {
      duration_t tmo = patt.timeout_;
      if (tmo > zero)
      {
        if (tmo < infin)
        {
          start_timer(tmo, f);
        }
        recv_h_ = f;
        curr_pattern_ = patt;
        return;
      }
    }
    else
    {
      sender = end_recv(rcv, msg);
    }

    self_ref_t aref(*this);
    base_t::snd_.post(
      boost::bind<void>(f, aref, sender, msg)
      );
  }

  void respond(
    recv_handler_t const& f, resp_t res,
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    aid_t sender;
    message msg;

    if (!base_t::mb_.pop(res, msg))
    {
      if (tmo > zero)
      {
        if (tmo < infin)
        {
          start_res_timer(tmo, f);
        }
        res_h_ = f;
        recving_res_ = res;
        return;
      }
    }
    else
    {
      sender = end_recv(res);
    }

    self_ref_t aref(*this);
    base_t::snd_.post(
      boost::bind<void>(f, aref, sender, msg)
      );
  }

  void sleep_for(wait_handler_t const& f, duration_t dur)
  {
    start_wait_timer(dur, f);
    wait_h_ = f;
  }

  void quit(
    exit_code_t exc = exit_normal,
    std::string const& errmsg = std::string()
    )
  {
    aid_t self_aid = base_t::get_aid();
    base_t::snd_.post(
      boost::bind(
        &self_t::stop, this,
        self_aid, exc, errmsg
        )
      );
  }

public:
  void init(func_t const& f)
  {
    f_ = f;
  }

  void start()
  {
    run();
  }

  void on_recv(pack& pk)
  {
    handle_recv(pk);
  }

  void on_addon_recv(pack& pk)
  {
    base_t::snd_.dispatch(
      boost::bind(
        &self_t::handle_recv, this, pk
        )
      );
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

  void spawn_handler(self_ref_t, aid_t sender, aid_t& osender)
  {
    osender = sender;
    run();
  }

  void run()
  {
    try
    {
      self_ref_t aref(*this);
      f_(aref);
      if (coro_.is_complete())
      {
        quit();
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      quit(exit_except, ex.what());
    }
  }

private:
  void stop(aid_t self_aid, exit_code_t ec, std::string const& exit_msg)
  {
    base_t::send_exit(self_aid, ec, exit_msg);
    svc_.free_actor(this);
  }

  void start_timer(duration_t dur, recv_handler_t const& hdr)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(
      base_t::snd_.wrap(
        boost::bind(
          &self_t::handle_recv_timeout, this,
          boost::asio::placeholders::error, ++tmr_sid_, hdr
          )
        )
      );
  }

  void start_res_timer(duration_t dur, recv_handler_t const& hdr)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(
      base_t::snd_.wrap(
        boost::bind(
          &self_t::handle_res_timeout, this,
          boost::asio::placeholders::error, ++tmr_sid_, hdr
          )
        )
      );
  }

  void start_wait_timer(duration_t dur, wait_handler_t const& hdr)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(
      base_t::snd_.wrap(
        boost::bind(
          &self_t::handle_wait_timeout, this,
          boost::asio::placeholders::error, ++tmr_sid_, hdr
          )
        )
      );
  }

  void handle_recv_timeout(
    errcode_t const& ec, size_t tmr_sid, recv_handler_t const& hdr
    )
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      GCE_ASSERT(recv_h_);
      recv_h_.clear();
      curr_pattern_.clear();
      try
      {
        self_ref_t aref(*this);
        hdr(aref, aid_t(), message());
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
        quit(exit_except, ex.what());
      }
    }
  }

  void handle_res_timeout(
    errcode_t const& ec, size_t tmr_sid, recv_handler_t const& hdr
    )
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      GCE_ASSERT(res_h_);
      res_h_.clear();
      curr_pattern_.clear();
      try
      {
        self_ref_t aref(*this);
        hdr(aref, aid_t(), message());
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
        quit(exit_except, ex.what());
      }
    }
  }

  void handle_wait_timeout(
    errcode_t const& ec, size_t tmr_sid, wait_handler_t const& hdr
    )
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      GCE_ASSERT(wait_h_);
      wait_h_.clear();
      try
      {
        self_ref_t aref(*this);
        hdr(aref);
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
        quit(exit_except, ex.what());
      }
    }
  }

  void handle_recv(pack& pk)
  {
    bool is_response = false;

    if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
    {
      base_t::mb_.push(*aid, pk.msg_);
    }
    else if (request_t* req = boost::get<request_t>(&pk.tag_))
    {
      base_t::mb_.push(*req, pk.msg_);
    }
    else if (link_t* link = boost::get<link_t>(&pk.tag_))
    {
      base_t::add_link(link->get_aid(), pk.skt_);
      return;
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      base_t::mb_.push(*ex, pk.msg_);
      base_t::remove_link(ex->get_aid());
    }
    else if (resp_t* res = boost::get<resp_t>(&pk.tag_))
    {
      is_response = true;
      base_t::mb_.push(*res, pk.msg_);
    }

    match_t ty = pk.msg_.get_type();
    recv_t rcv;
    message msg;
    aid_t sender;
    recv_handler_t hdr;

    if (
      (recv_h_ && !is_response) ||
      (res_h_ && (is_response || ty == exit))
      )
    {
      if (recv_h_ && !is_response)
      {
        bool ret = base_t::mb_.pop(rcv, msg, curr_pattern_.match_list_, curr_pattern_.recver_);
        if (!ret)
        {
          return;
        }
        sender = end_recv(rcv, msg);
        curr_pattern_.clear();
        hdr = recv_h_;
        recv_h_.clear();
      }

      if (res_h_ && (is_response || ty == exit))
      {
        GCE_ASSERT(recving_res_.valid());
        bool ret = base_t::mb_.pop(recving_res_, msg);
        if (!ret)
        {
          return;
        }
        sender = end_recv(recving_res_);
        hdr = res_h_;
        res_h_.clear();
        recving_res_ = resp_t();
      }

      ++tmr_sid_;
      errcode_t ec;
      tmr_.cancel(ec);

      if (hdr)
      {
        try
        {
          self_ref_t aref(*this);
          hdr(aref, sender, msg);
        }
        catch (std::exception& ex)
        {
          GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
          quit(exit_except, ex.what());
        }
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

  void recv_handler(
    self_ref_t, aid_t sender, message msg, aid_t& osender, message& omsg
    )
  {
    osender = sender;
    omsg = msg;
    run();
  }

  void wait_handler(self_ref_t)
  {
    run();
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(func_t, f_)
  GCE_CACHE_ALIGNED_VAR(coro_t, coro_)

  /// coro local vars
  service_t& svc_;
  recv_handler_t recv_h_;
  recv_handler_t res_h_;
  wait_handler_t wait_h_;
  resp_t recving_res_;
  pattern curr_pattern_;
  timer_t tmr_;
  size_t tmr_sid_;
  log::logger_t& lg_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_STACKLESS_ACTOR_HPP
