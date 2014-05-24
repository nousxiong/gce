///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/event_based_actor.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
event_based_actor::event_based_actor(detail::cache_pool* user)
  : base_type(&user->get_context(), user, user->get_index())
  , stat_(ready)
  , tmr_(ctx_->get_io_service())
  , tmr_sid_(0)
{
}
///----------------------------------------------------------------------------
event_based_actor::~event_based_actor()
{
}
///----------------------------------------------------------------------------
void event_based_actor::recv(event_based_actor::recv_handler_t const& f, match const& mach)
{
  aid_t sender;
  detail::recv_t rcv;
  message msg;

  if (!mb_.pop(rcv, msg, mach.match_list_))
  {
    duration_t tmo = mach.timeout_;
    if (tmo > zero)
    {
      if (tmo < infin)
      {
        start_recv_timer(tmo, f);
      }
      recv_h_ = f;
      curr_match_ = mach;
      return;
    }
  }
  else
  {
    sender = end_recv(rcv, msg);
  }

  actor<evented> aref(*this);
  snd_.post(
    boost::bind<void>(f, aref, sender, msg)
    );
}
///----------------------------------------------------------------------------
void event_based_actor::recv(event_based_actor::recv_handler_t const& f, response_t res, duration_t tmo)
{
  aid_t sender;
  message msg;

  if (!mb_.pop(res, msg))
  {
    if (tmo > zero)
    {
      if (tmo < infin)
      {
        start_recv_timer(tmo, f);
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

  actor<evented> aref(*this);
  snd_.post(
    boost::bind<void>(f, aref, sender, msg)
    );
}
///----------------------------------------------------------------------------
void event_based_actor::wait(wait_handler_t const& f, duration_t dur)
{
  start_wait_timer(dur, f);
  wait_h_ = f;
}
///----------------------------------------------------------------------------
void event_based_actor::quit(exit_code_t exc, std::string const& errmsg)
{
  aid_t self_aid = get_aid();
  base_type::update_aid();
  snd_.post(boost::bind(&event_based_actor::stop, this, self_aid, exc, errmsg));
}
///----------------------------------------------------------------------------
void event_based_actor::init()
{
  BOOST_ASSERT_MSG(stat_ == ready, "event_based_actor status error");
  base_type::update_aid();
}
///----------------------------------------------------------------------------
void event_based_actor::start(event_based_actor::func_t const& f)
{
  snd_.post(boost::bind(&event_based_actor::run, this, f));
}
///----------------------------------------------------------------------------
void event_based_actor::on_free()
{
  base_type::on_free();

  stat_ = ready;
  curr_match_.clear();

  recv_h_.clear();
  res_h_.clear();
  wait_h_.clear();
  recving_res_ = response_t();
}
///----------------------------------------------------------------------------
void event_based_actor::on_recv(detail::pack& pk, base_type::send_hint hint)
{
  if (hint == base_type::sync)
  {
    snd_.dispatch(
      boost::bind(
        &event_based_actor::handle_recv, this, pk
        )
      );
  }
  else
  {
    snd_.post(
      boost::bind(
        &event_based_actor::handle_recv, this, pk
        )
      );
  }
}
///----------------------------------------------------------------------------
void event_based_actor::run(func_t const& f)
{
  try
  {
    stat_ = on;
    actor<evented> aref(*this);
    f(aref);
  }
  catch (std::exception& ex)
  {
    quit(exit_except, ex.what());
  }
}
///----------------------------------------------------------------------------
void event_based_actor::stop(aid_t self_aid, exit_code_t ec, std::string const& exit_msg)
{
  stat_ = off;
  base_type::send_exit(self_aid, ec, exit_msg);
  user_->free_actor(this);
}
///----------------------------------------------------------------------------
void event_based_actor::start_recv_timer(duration_t dur, recv_handler_t const& hdr)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &event_based_actor::handle_recv_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_, hdr
        )
      )
    );
}
///----------------------------------------------------------------------------
void event_based_actor::start_res_timer(duration_t dur, recv_handler_t const& hdr)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &event_based_actor::handle_res_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_, hdr
        )
      )
    );
}
///----------------------------------------------------------------------------
void event_based_actor::start_wait_timer(duration_t dur, wait_handler_t const& hdr)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &event_based_actor::handle_wait_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_, hdr
        )
      )
    );
}
///----------------------------------------------------------------------------
void event_based_actor::handle_recv_timeout(
  errcode_t const& ec, std::size_t tmr_sid, recv_handler_t const& hdr
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    BOOST_ASSERT(recv_h_);
    recv_h_.clear();
    curr_match_.clear();
    try
    {
      actor<evented> aref(*this);
      hdr(aref, aid_t(), message());
    }
    catch (std::exception& ex)
    {
      quit(exit_except, ex.what());
    }
  }
}
///----------------------------------------------------------------------------
void event_based_actor::handle_res_timeout(
  errcode_t const& ec, std::size_t tmr_sid, recv_handler_t const& hdr
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    BOOST_ASSERT(res_h_);
    res_h_.clear();
    curr_match_.clear();
    try
    {
      actor<evented> aref(*this);
      hdr(aref, aid_t(), message());
    }
    catch (std::exception& ex)
    {
      quit(exit_except, ex.what());
    }
  }
}
///----------------------------------------------------------------------------
void event_based_actor::handle_wait_timeout(
  errcode_t const& ec, std::size_t tmr_sid, wait_handler_t const& hdr
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    BOOST_ASSERT(wait_h_);
    wait_h_.clear();
    try
    {
      actor<evented> aref(*this);
      hdr(aref);
    }
    catch (std::exception& ex)
    {
      quit(exit_except, ex.what());
    }
  }
}
///----------------------------------------------------------------------------
void event_based_actor::handle_recv(detail::pack& pk)
{
  if (check(pk.recver_, get_aid().ctxid_, user_->get_context().get_timestamp()))
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

    detail::recv_t rcv;
    message msg;
    aid_t sender;
    recv_handler_t hdr;

    if (
      (recv_h_ && !is_response) ||
      (res_h_ && is_response)
      )
    {
      if (recv_h_ && !is_response)
      {
        bool ret = mb_.pop(rcv, msg, curr_match_.match_list_);
        if (!ret)
        {
          return;
        }
        sender = end_recv(rcv, msg);
        curr_match_.clear();
        hdr = recv_h_;
        recv_h_.clear();
      }

      if (res_h_ && is_response)
      {
        BOOST_ASSERT(recving_res_.valid());
        bool ret = mb_.pop(recving_res_, msg);
        if (!ret)
        {
          return;
        }
        sender = end_recv(recving_res_);
        hdr = res_h_;
        res_h_.clear();
        recving_res_ = response_t();
      }

      ++tmr_sid_;
      errcode_t ec;
      tmr_.cancel(ec);

      if (hdr)
      {
        try
        {
          actor<evented> aref(*this);
          hdr(aref, sender, msg);
        }
        catch (std::exception& ex)
        {
          quit(exit_except, ex.what());
        }
      }
    }
  }
  else if (!pk.is_err_ret_)
  {
    if (detail::link_t* link = boost::get<detail::link_t>(&pk.tag_))
    {
      /// send event exit msg
      base_type::send_already_exited(link->get_aid(), pk.recver_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk.tag_))
    {
      /// reply event exit msg
      response_t res(req->get_id(), pk.recver_);
      base_type::send_already_exited(req->get_aid(), res);
    }
  }
}
///----------------------------------------------------------------------------
aid_t event_based_actor::end_recv(detail::recv_t& rcv, message& msg)
{
  aid_t sender;
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
aid_t event_based_actor::end_recv(response_t& res)
{
  return res.get_aid();
}
///----------------------------------------------------------------------------
}
