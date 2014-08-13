///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/coroutine_stackless_actor.hpp>
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
coroutine_stackless_actor::coroutine_stackless_actor(aid_t aid, detail::cache_pool* user)
  : base_type(&user->get_context(), user, aid, user->get_index())
  , tmr_(ctx_->get_io_service())
  , tmr_sid_(0)
{
}
///----------------------------------------------------------------------------
coroutine_stackless_actor::~coroutine_stackless_actor()
{
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::recv(aid_t& sender, message& msg, pattern const& patt)
{
  recv(
    boost::bind(
      &coroutine_stackless_actor::recv_handler, this, _1, _2, _3, 
      boost::ref(sender), boost::ref(msg)
      ), 
    patt
    );
}
///----------------------------------------------------------------------------
aid_t coroutine_stackless_actor::recv(message& msg, match_list_t const& match_list)
{
  aid_t sender;
  detail::recv_t rcv;

  if (mb_.pop(rcv, msg, match_list))
  {
    sender = end_recv(rcv, msg);
  }

  return sender;
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::recv(resp_t res, aid_t& sender, message& msg, duration_t tmo)
{
  recv(
    boost::bind(
      &coroutine_stackless_actor::recv_handler, this, _1, _2, _3, 
      boost::ref(sender), boost::ref(msg)
      ), 
    res,
    tmo
    );
}
///----------------------------------------------------------------------------
aid_t coroutine_stackless_actor::recv(resp_t res, message& msg)
{
  aid_t sender;

  if (mb_.pop(res, msg))
  {
    sender = end_recv(res);
  }

  return sender;
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::wait(duration_t dur)
{
  wait(
    boost::bind(
      &coroutine_stackless_actor::wait_handler, this, _1
      ),
    dur
    );
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::recv(
  coroutine_stackless_actor::recv_handler_t const& f, pattern const& patt
  )
{
  aid_t sender;
  detail::recv_t rcv;
  message msg;

  if (!mb_.pop(rcv, msg, patt.match_list_))
  {
    duration_t tmo = patt.timeout_;
    if (tmo > zero)
    {
      if (tmo < infin)
      {
        start_recv_timer(tmo, f);
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

  actor<stackless> aref(*this);
  snd_.post(
    boost::bind<void>(f, aref, sender, msg)
    );
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::recv(
  coroutine_stackless_actor::recv_handler_t const& f, resp_t res, duration_t tmo
  )
{
  aid_t sender;
  message msg;

  if (!mb_.pop(res, msg))
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

  actor<stackless> aref(*this);
  snd_.post(
    boost::bind<void>(f, aref, sender, msg)
    );
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::wait(wait_handler_t const& f, duration_t dur)
{
  start_wait_timer(dur, f);
  wait_h_ = f;
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::quit(exit_code_t exc, std::string const& errmsg)
{
  aid_t self_aid = get_aid();
  snd_.post(boost::bind(&coroutine_stackless_actor::stop, this, self_aid, exc, errmsg));
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::init(coroutine_stackless_actor::func_t const& f)
{
  f_ = f;
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::start()
{
  run();
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::on_recv(detail::pack& pk, detail::send_hint)
{
  handle_recv(pk);
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::spawn_handler(self_ref_t, aid_t sender, aid_t& osender)
{
  osender = sender;
  run();
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::run()
{
  try
  {
    actor<stackless> aref(*this);
    f_(aref);
    if (coro_.is_complete())
    {
      quit();
    }
  }
  catch (std::exception& ex)
  {
    quit(exit_except, ex.what());
  }
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::stop(aid_t self_aid, exit_code_t ec, std::string const& exit_msg)
{
  base_type::send_exit(self_aid, ec, exit_msg);
  user_->free_actor(this);
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::start_recv_timer(duration_t dur, recv_handler_t const& hdr)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &coroutine_stackless_actor::handle_recv_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_, hdr
        )
      )
    );
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::start_res_timer(duration_t dur, recv_handler_t const& hdr)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &coroutine_stackless_actor::handle_res_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_, hdr
        )
      )
    );
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::start_wait_timer(duration_t dur, wait_handler_t const& hdr)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &coroutine_stackless_actor::handle_wait_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_, hdr
        )
      )
    );
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::handle_recv_timeout(
  errcode_t const& ec, std::size_t tmr_sid, recv_handler_t const& hdr
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    BOOST_ASSERT(recv_h_);
    recv_h_.clear();
    curr_pattern_.clear();
    try
    {
      actor<stackless> aref(*this);
      hdr(aref, aid_t(), message());
    }
    catch (std::exception& ex)
    {
      quit(exit_except, ex.what());
    }
  }
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::handle_res_timeout(
  errcode_t const& ec, std::size_t tmr_sid, recv_handler_t const& hdr
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    BOOST_ASSERT(res_h_);
    res_h_.clear();
    curr_pattern_.clear();
    try
    {
      actor<stackless> aref(*this);
      hdr(aref, aid_t(), message());
    }
    catch (std::exception& ex)
    {
      quit(exit_except, ex.what());
    }
  }
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::handle_wait_timeout(
  errcode_t const& ec, std::size_t tmr_sid, wait_handler_t const& hdr
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    BOOST_ASSERT(wait_h_);
    wait_h_.clear();
    try
    {
      actor<stackless> aref(*this);
      hdr(aref);
    }
    catch (std::exception& ex)
    {
      quit(exit_except, ex.what());
    }
  }
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::handle_recv(detail::pack& pk)
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
  else if (resp_t* res = boost::get<resp_t>(&pk.tag_))
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
      bool ret = mb_.pop(rcv, msg, curr_pattern_.match_list_);
      if (!ret)
      {
        return;
      }
      sender = end_recv(rcv, msg);
      curr_pattern_.clear();
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
      recving_res_ = resp_t();
    }

    ++tmr_sid_;
    errcode_t ec;
    tmr_.cancel(ec);

    if (hdr)
    {
      try
      {
        actor<stackless> aref(*this);
        hdr(aref, sender, msg);
      }
      catch (std::exception& ex)
      {
        quit(exit_except, ex.what());
      }
    }
  }
}
///----------------------------------------------------------------------------
aid_t coroutine_stackless_actor::end_recv(detail::recv_t& rcv, message& msg)
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
aid_t coroutine_stackless_actor::end_recv(resp_t& res)
{
  return res.get_aid();
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::recv_handler(
  self_ref_t, aid_t sender, message msg, aid_t& osender, message& omsg
  )
{
  osender = sender;
  omsg = msg;
  run();
}
///----------------------------------------------------------------------------
void coroutine_stackless_actor::wait_handler(self_ref_t)
{
  run();
}
///----------------------------------------------------------------------------
}
