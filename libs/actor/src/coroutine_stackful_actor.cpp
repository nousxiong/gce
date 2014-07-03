///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/coroutine_stackful_actor.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/thread_mapped_actor.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/detail/scoped_bool.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
coroutine_stackful_actor::coroutine_stackful_actor(detail::cache_pool* user)
  : base_type(&user->get_context(), user, user->get_index())
  , stat_(ready)
  , recving_(false)
  , responsing_(false)
  , tmr_(ctx_->get_io_service())
  , tmr_sid_(0)
  , yld_(0)
{
}
///----------------------------------------------------------------------------
coroutine_stackful_actor::~coroutine_stackful_actor()
{
}
///----------------------------------------------------------------------------
aid_t coroutine_stackful_actor::recv(message& msg, match const& mach)
{
  aid_t sender;
  detail::recv_t rcv;

  if (!mb_.pop(rcv, msg, mach.match_list_))
  {
    duration_t tmo = mach.timeout_;
    if (tmo > zero)
    {
      detail::scoped_bool<bool> scp(recving_);
      if (tmo < infin)
      {
        start_recv_timer(tmo);
      }
      curr_match_ = mach;
      actor_code ac = yield();
      if (ac == actor_timeout)
      {
        return sender;
      }

      rcv = recving_rcv_;
      msg = recving_msg_;
      recving_rcv_ = detail::recv_t();
      recving_msg_ = message();
    }
    else
    {
      return sender;
    }
  }

  if (aid_t* aid = boost::get<aid_t>(&rcv))
  {
    sender = *aid;
  }
  else if (detail::request_t* req = boost::get<detail::request_t>(&rcv))
  {
    sender = req->get_aid();
    msg.req_ = *req;
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv))
  {
    sender = ex->get_aid();
  }

  return sender;
}
///----------------------------------------------------------------------------
aid_t coroutine_stackful_actor::recv(response_t res, message& msg, duration_t tmo)
{
  aid_t sender;

  recving_res_ = res;
  if (!mb_.pop(res, msg))
  {
    if (tmo > zero)
    {
      detail::scoped_bool<bool> scp(responsing_);
      if (tmo < infin)
      {
        start_recv_timer(tmo);
      }
      actor_code ac = yield();
      if (ac == actor_timeout)
      {
        return sender;
      }

      res = recving_res_;
      msg = recving_msg_;
      recving_res_ = response_t();
      recving_msg_ = message();
    }
    else
    {
      return sender;
    }
  }

  sender = res.get_aid();
  return sender;
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::wait(duration_t dur)
{
  start_recv_timer(dur);
  yield();
}
///----------------------------------------------------------------------------
yield_t coroutine_stackful_actor::get_yield()
{
  BOOST_ASSERT(yld_);
  return *yld_;
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::start(std::size_t stack_size)
{
  if (stack_size < minimum_stacksize())
  {
    stack_size = minimum_stacksize();
  }
  else if (stack_size > default_stacksize())
  {
    stack_size = default_stacksize();
  }

  boost::asio::spawn(
    snd_,
    boost::bind(
      &coroutine_stackful_actor::run, this, _1
      ),
    boost::coroutines::attributes(stack_size)
    );
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::init(coroutine_stackful_actor::func_t const& f)
{
  BOOST_ASSERT_MSG(stat_ == ready, "coroutine_stackful_actor status error");
  f_ = f;
  base_type::update_aid();
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::on_free()
{
  base_type::on_free();

  stat_ = ready;
  f_.clear();
  curr_match_.clear();

  recving_ = false;
  responsing_ = false;
  recving_rcv_ = detail::recv_t();
  recving_res_ = response_t();
  recving_msg_ = message();
  ec_ = exit_normal;
  exit_msg_.clear();
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::on_recv(detail::pack& pk, base_type::send_hint hint)
{
  if (hint == base_type::sync)
  {
    snd_.dispatch(
      boost::bind(
        &coroutine_stackful_actor::handle_recv, this, pk
        )
      );
  }
  else
  {
    snd_.post(
      boost::bind(
        &coroutine_stackful_actor::handle_recv, this, pk
        )
      );
  }
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::run(yield_t yld)
{
  yld_ = &yld;

  try
  {
    stat_ = on;
    actor<stackful> aref(*this);
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
    stop(exit_except, ex.what());
  }
  catch (...)
  {
    stop(exit_except, "unexpected exception");
  }
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::resume(actor_code ac)
{
  detail::scope scp(boost::bind(&coroutine_stackful_actor::free_self, this));
  BOOST_ASSERT(yld_cb_);
  yld_cb_(ac);

  if (stat_ != off)
  {
    scp.reset();
  }
}
///----------------------------------------------------------------------------
coroutine_stackful_actor::actor_code coroutine_stackful_actor::yield()
{
  BOOST_ASSERT(yld_);
  boost::asio::detail::async_result_init<
    yield_t, void (actor_code)> init(
      BOOST_ASIO_MOVE_CAST(yield_t)(*yld_));

  yld_cb_ = boost::bind<void>(init.handler, _1);
  return init.result.get();
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::free_self()
{
  aid_t self_aid = get_aid();
  base_type::update_aid();
  base_type::send_exit(self_aid, ec_, exit_msg_);
  user_->free_actor(this);
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::stop(exit_code_t ec, std::string exit_msg)
{
  if (ec == exit_normal)
  {
    /// Trigger a context switching, ensure we stop coro using coroutine_stackful_actor::resume.
    snd_.post(boost::bind(&coroutine_stackful_actor::resume, this, actor_normal));
    yield();
  }

  stat_ = off;
  ec_ = ec;
  exit_msg_ = exit_msg;
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::start_recv_timer(duration_t dur)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &coroutine_stackful_actor::handle_recv_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_
        )
      )
    );
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::handle_recv_timeout(errcode_t const& ec, std::size_t tmr_sid)
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    resume(actor_timeout);
  }
}
///----------------------------------------------------------------------------
void coroutine_stackful_actor::handle_recv(detail::pack& pk)
{
  if (check(pk.recver_, ctxid_, timestamp_))
  {
    bool is_response = false;

    if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
    {
      mb_.push(*aid, pk.msg_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk.tag_))
    {
      mb_.push(*req, pk.msg_);
    }
    else if (detail::link_t* link = boost::get<detail::link_t>(&pk.tag_))
    {
      add_link(link->get_aid(), pk.skt_);
      return;
    }
    else if (detail::exit_t* ex = boost::get<detail::exit_t>(&pk.tag_))
    {
      mb_.push(*ex, pk.msg_);
      base_type::remove_link(ex->get_aid());
    }
    else if (response_t* res = boost::get<response_t>(&pk.tag_))
    {
      is_response = true;
      mb_.push(*res, pk.msg_);
    }

    if (
      (recving_ && !is_response) ||
      (responsing_ && is_response)
      )
    {
      if (recving_ && !is_response)
      {
        bool ret = mb_.pop(recving_rcv_, recving_msg_, curr_match_.match_list_);
        if (!ret)
        {
          return;
        }
        curr_match_.clear();
      }

      if (responsing_ && is_response)
      {
        BOOST_ASSERT(recving_res_.valid());
        bool ret = mb_.pop(recving_res_, recving_msg_);
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
  else if (!pk.is_err_ret_)
  {
    if (detail::link_t* link = boost::get<detail::link_t>(&pk.tag_))
    {
      /// send actor exit msg
      base_type::send_already_exited(link->get_aid(), pk.recver_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk.tag_))
    {
      /// reply actor exit msg
      response_t res(req->get_id(), pk.recver_);
      base_type::send_already_exited(req->get_aid(), res);
    }
  }
}
///----------------------------------------------------------------------------
}
