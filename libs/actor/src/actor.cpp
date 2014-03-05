///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/actor.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/detail/scoped_bool.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
actor::actor(context* ctx)
  : base_type(ctx->get_attributes().max_cache_match_size_, ctx->get_timestamp())
  , stat_(ready)
  , self_(*this)
  , recving_(false)
  , responsing_(false)
  , tmr_(ctx->get_io_service())
  , tmr_sid_(0)
  , yld_(0)
{
}
///----------------------------------------------------------------------------
actor::~actor()
{
}
///----------------------------------------------------------------------------
aid_t actor::recv(message& msg, match const& mach)
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
aid_t actor::recv(response_t res, message& msg, duration_t tmo)
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
void actor::wait(duration_t dur)
{
  if (dur < infin)
  {
    start_recv_timer(dur);
  }
  yield();
}
///----------------------------------------------------------------------------
void actor::link(aid_t target)
{
  base_type::link(detail::link_t(linked, target), user_);
}
///----------------------------------------------------------------------------
void actor::monitor(aid_t target)
{
  base_type::link(detail::link_t(monitored, target), user_);
}
///----------------------------------------------------------------------------
detail::cache_pool* actor::get_cache_pool()
{
  return user_;
}
///----------------------------------------------------------------------------
yield_t actor::get_yield()
{
  BOOST_ASSERT(yld_);
  return *yld_;
}
///----------------------------------------------------------------------------
void actor::start(std::size_t stack_size)
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
    user_->get_strand(),
    boost::bind(
      &actor::run, this, _1
      ),
    boost::coroutines::attributes(stack_size)
    );
}
///----------------------------------------------------------------------------
void actor::init(
  detail::cache_pool* user, detail::cache_pool* owner,
  actor_func_t const& f,
  aid_t link_tgt
  )
{
  BOOST_ASSERT_MSG(stat_ == ready, "actor status error");
  user_ = user;
  owner_ = owner;
  f_ = f;
  base_type::update_aid(user_->get_ctxid());

  if (link_tgt)
  {
    base_type::add_link(link_tgt);
  }
}
///----------------------------------------------------------------------------
void actor::on_free()
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
void actor::on_recv(detail::pack* pk)
{
  user_->get_strand().dispatch(
    boost::bind(
      &actor::handle_recv, this, pk
      )
    );
}
///----------------------------------------------------------------------------
void actor::run(yield_t yld)
{
  yld_ = &yld;

  try
  {
    stat_ = on;
    f_(self_);
    stop(exit_normal, "exit normal");
  }
  catch (boost::coroutines::detail::forced_unwind const&)
  {
    stop(exit_normal, "exit normal");
    throw;
  }
  catch (std::exception& ex)
  {
    stop(exit_except, ex.what());
  }
  catch (...)
  {
    stop(exit_unknown, "unexpected exception");
  }
}
///----------------------------------------------------------------------------
void actor::resume(actor_code ac)
{
  detail::scope scp(boost::bind(&actor::free_self, this));
  BOOST_ASSERT(yld_cb_);
  yld_cb_(ac);

  if (stat_ != off)
  {
    scp.reset();
  }
}
///----------------------------------------------------------------------------
actor::actor_code actor::yield()
{
  BOOST_ASSERT(yld_);
  boost::asio::detail::async_result_init<
    yield_t, void (actor_code)> init(
      BOOST_ASIO_MOVE_CAST(yield_t)(*yld_));

  yld_cb_ = boost::bind<void>(init.handler, _1);
  return init.result.get();
}
///----------------------------------------------------------------------------
void actor::free_self()
{
  base_type::send_exit(ec_, exit_msg_, user_);
  base_type::update_aid(user_->get_ctxid());
  user_->free_actor(owner_, this);
}
///----------------------------------------------------------------------------
void actor::stop(exit_code_t ec, std::string const& exit_msg)
{
  /// Trigger a context switch, ensure we stop coro using actor::resume.
  user_->get_strand().post(boost::bind(&actor::resume, this, actor_normal));
  yield();

  stat_ = off;
  ec_ = ec;
  exit_msg_ = exit_msg;
}
///----------------------------------------------------------------------------
void actor::start_recv_timer(duration_t dur)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    user_->get_strand().wrap(
      boost::bind(
        &actor::handle_recv_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_
        )
      )
    );
}
///----------------------------------------------------------------------------
void actor::handle_recv_timeout(errcode_t const& ec, std::size_t tmr_sid)
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    resume(actor_timeout);
  }
}
///----------------------------------------------------------------------------
void actor::handle_recv(detail::pack* pk)
{
  detail::scope scp(boost::bind(&basic_actor::dealloc_pack, user_, pk));
  if (check(pk->recver_, get_aid().ctxid_, user_->get_context().get_timestamp()))
  {
    bool is_response = false;

    if (aid_t* aid = boost::get<aid_t>(&pk->tag_))
    {
      mb_.push(*aid, pk->msg_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
    {
      mb_.push(*req, pk->msg_);
    }
    else if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
    {
      add_link(link->get_aid());
      return;
    }
    else if (detail::exit_t* ex = boost::get<detail::exit_t>(&pk->tag_))
    {
      mb_.push(*ex, pk->msg_);
      base_type::remove_link(ex->get_aid());
    }
    else if (response_t* res = boost::get<response_t>(&pk->tag_))
    {
      is_response = true;
      mb_.push(*res, pk->msg_);
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
  else if (!pk->is_err_ret_)
  {
    if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
    {
      /// send actor exit msg
      base_type::send_already_exited(link->get_aid(), pk->recver_, user_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
    {
      /// reply actor exit msg
      response_t res(req->get_id(), pk->recver_);
      base_type::send_already_exited(req->get_aid(), res, user_);
    }
  }
}
///----------------------------------------------------------------------------
}
