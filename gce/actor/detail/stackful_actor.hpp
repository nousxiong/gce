///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_STACKFUL_ACTOR_HPP
#define GCE_ACTOR_DETAIL_STACKFUL_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/actor_service.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/actor/detail/actor_ref.hpp>
#include <gce/actor/detail/scoped_bool.hpp>
#include <gce/actor/pattern.hpp>
#include <gce/actor/attributes.hpp>
#include <boost/coroutine/exceptions.hpp>
#include <boost/optional.hpp>

namespace gce
{
namespace detail
{
template <typename Context>
class stackful_actor
  : public basic_actor<Context>
{
public:
  typedef Context context_t;

private:
  typedef basic_actor<context_t> base_t;
  typedef stackful_actor<context_t> self_t;
  typedef actor_service<self_t, true> service_t;

  enum actor_code
  {
    actor_normal = 0,
    actor_timeout,
  };

  enum status
  {
    ready = 0,
    on,
    off
  };

  typedef boost::asio::detail::async_result_init<yield_t, void (actor_code)> async_result_init_t;

public:
  typedef actor_ref<stackful, context_t> self_ref_t;
  typedef boost::function<void (self_ref_t)> func_t;

public:
  stackful_actor(aid_t aid, service_t& svc)
    : base_t(svc.get_context(), svc, actor_stackful, aid)
    , stat_(ready)
    , svc_(svc)
    , recving_(false)
    , responsing_(false)
    , tmr_(base_t::ctx_.get_io_service())
    , tmr_sid_(0)
    , ec_(exit_normal)
    , lg_(base_t::ctx_.get_logger())
    , guard_(base_t::ctx_.get_io_service())
  {
    guard_.expires_from_now(to_chrono(infin));
  }

  ~stackful_actor()
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

  aid_t recv(message& msg, pattern const& patt = pattern())
  {
    aid_t sender;
    recv_t rcv;

    if (!base_t::mb_.pop(rcv, msg, patt.match_list_, patt.recver_))
    {
      duration_t tmo = patt.timeout_;
      if (tmo >= zero) /// change > to >= for yielding when timeout == 0
      {
        scoped_bool<bool> scp(recving_);
        if (tmo < infin)
        {
          start_timer(tmo);
        }
        curr_pattern_ = patt;
        actor_code ac = yield();
        if (ac == actor_timeout)
        {
          return sender;
        }

        rcv = recving_rcv_;
        msg = recving_msg_;
        recving_rcv_ = nil_rcv_;
        recving_msg_ = nil_msg_;
      }
      else
      {
        return sender;
      }
    }

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

  aid_t respond(
    resp_t res, message& msg, 
    duration_t tmo = seconds(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    aid_t sender;

    recving_res_ = res;
    if (!base_t::mb_.pop(res, msg))
    {
      if (tmo >= zero) /// change > to >= for yielding when timeout == 0
      {
        scoped_bool<bool> scp(responsing_);
        if (tmo < infin)
        {
          start_timer(tmo);
        }
        actor_code ac = yield();
        if (ac == actor_timeout)
        {
          return sender;
        }

        res = recving_res_;
        msg = recving_msg_;
        recving_res_ = nil_resp_;
        recving_msg_ = nil_msg_;
      }
      else
      {
        return sender;
      }
    }

    sender = res.get_aid();
    return sender;
  }

  void sleep_for(duration_t dur)
  {
    start_timer(dur);
    yield();
  }

  yield_t get_yield()
  {
    GCE_ASSERT(yld_);
    return *yld_;
  }

public:
  /// internal use
  typedef gce::stackful type;

  static actor_type get_type()
  {
    return actor_stackful;
  }

  static size_t get_pool_reserve_size(attributes const& attr)
  {
    return attr.actor_pool_reserve_size_;
  }

  service_t& get_service()
  {
    return svc_;
  }

  void start(size_t stack_size)
  {
    guard_.async_wait(boost::bind(&self_t::guard));

    if (stack_size < minimum_stacksize())
    {
      stack_size = minimum_stacksize();
    }

    boost::asio::spawn(
      base_t::snd_,
      boost::bind(
        &self_t::run, this, _arg1
        ),
      boost::coroutines::attributes(stack_size)
      );
  }

  void init(func_t const& f)
  {
    GCE_ASSERT(stat_ == ready)(stat_).log(lg_, "stackful_actor status error");
    f_ = f;
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

  sid_t spawn(spawn_type type, std::string const& func, match_t ctxid, size_t stack_size)
  {
    sid_t sid = base_t::new_request();
    base_t::pri_spawn(sid, type, func, ctxid, stack_size);
    return sid;
  }

private:
  void run(yield_t yld)
  {
    yld_ = boost::in_place(yld);

    try
    {
      stat_ = on;
      self_ref_t aref(*this);
      f_(aref);
      stop(exit_normal, "exit normal");
    }
    catch (boost::coroutines::detail::forced_unwind const&)
    {
      stop(exit_except, "exit normal");
      throw;
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << ex.what();
      stop(exit_except, ex.what());
    }
    catch (...)
    {
      std::string errmsg = boost::current_exception_diagnostic_information();
      GCE_ERROR(lg_)(__FILE__)(__LINE__) << errmsg;
      stop(exit_except, errmsg);
    }
  }

  void resume(actor_code ac = actor_normal)
  {
    scope scp(boost::bind(&self_t::free_self, this));
    GCE_ASSERT(yld_cb_);
    yld_cb_(ac);

    if (stat_ != off)
    {
      scp.reset();
    }
  }

  actor_code yield()
  {
    GCE_ASSERT(yld_);
    async_result_init_t init(BOOST_ASIO_MOVE_CAST(yield_t)(*yld_));

    yld_cb_ = boost::bind<void>(init.handler, _arg1);
    return init.result.get();
  }

  void free_self()
  {
    errcode_t ignored_ec;
    guard_.cancel(ignored_ec);
    aid_t self_aid = base_t::get_aid();
    base_t::send_exit(self_aid, ec_, exit_msg_);
    svc_.free_actor(this);
  }

  void stop(exit_code_t ec, std::string exit_msg)
  {
    if (ec == exit_normal)
    {
      /// Trigger a context switching, ensure we stop coro using self_t::resume.
      base_t::snd_.post(boost::bind(&self_t::resume, this, actor_normal));
      yield();
    }

    stat_ = off;
    ec_ = ec;
    exit_msg_ = exit_msg;

    if (ec != exit_normal)
    {
      free_self();
    }
  }

  void start_timer(duration_t dur)
  {
    tmr_.expires_from_now(to_chrono(dur));
    tmr_.async_wait(
      base_t::snd_.wrap(
        boost::bind(
          &self_t::handle_recv_timeout, this,
          boost::asio::placeholders::error, ++tmr_sid_
          )
        )
      );
  }

  void handle_recv_timeout(errcode_t const& ec, size_t tmr_sid)
  {
    if (!ec && tmr_sid == tmr_sid_)
    {
      resume(actor_timeout);
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

    if (
      (recving_ && !is_response) ||
      (responsing_ && (is_response || ty == exit))
      )
    {
      if (recving_ && !is_response)
      {
        bool ret = base_t::mb_.pop(recving_rcv_, recving_msg_, curr_pattern_.match_list_, curr_pattern_.recver_);
        if (!ret)
        {
          return;
        }
        curr_pattern_.clear();
      }

      if (responsing_ && (is_response || ty == exit))
      {
        GCE_ASSERT(recving_res_.valid());
        bool ret = base_t::mb_.pop(recving_res_, recving_msg_);
        if (!ret)
        {
          return;
        }
      }

      ++tmr_sid_;
      errcode_t ec;
      tmr_.cancel(ec);
      resume(actor_normal);
    }
  }

  static void guard() {}

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(func_t, f_)

  typedef boost::function<void (actor_code)> yield_cb_t;

  /// coro local vars
  service_t& svc_;
  bool recving_;
  bool responsing_;
  recv_t recving_rcv_;
  resp_t recving_res_;
  message recving_msg_;
  pattern curr_pattern_;
  timer_t tmr_;
  size_t tmr_sid_;
  boost::optional<yield_t> yld_;
  yield_cb_t yld_cb_;
  exit_code_t ec_;
  std::string exit_msg_;

  recv_t const nil_rcv_;
  resp_t const nil_resp_;
  message const nil_msg_;
  log::logger_t& lg_;

  timer_t guard_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_STACKFUL_ACTOR_HPP
