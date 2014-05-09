///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/mixin.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
mixin::mixin(detail::cache_pool* user)
  : base_type(&user->get_context(), user, user->get_index())
  , recv_p_(0)
  , res_p_(0)
  , tmr_(ctx_->get_io_service())
  , tmr_sid_(0)
{
  base_type::update_aid();
}
///----------------------------------------------------------------------------
mixin::~mixin()
{
}
///----------------------------------------------------------------------------
void mixin::send(aid_t recver, message const& m)
{
  snd_.post(
    boost::bind(
      &base_type::pri_send, this, recver, m, base_type::sync
      )
    );
}
///----------------------------------------------------------------------------
void mixin::send(svcid_t recver, message const& m)
{
  snd_.post(
    boost::bind(
      &base_type::pri_send_svc, this, recver, m, base_type::sync
      )
    );
}
///----------------------------------------------------------------------------
void mixin::relay(aid_t des, message& m)
{
  snd_.post(
    boost::bind(
      &base_type::pri_relay, this, des, m, base_type::sync
      )
    );
}
///----------------------------------------------------------------------------
void mixin::relay(svcid_t des, message& m)
{
  snd_.post(
    boost::bind(
      &base_type::pri_relay_svc, this, des, m, base_type::sync
      )
    );
}
///----------------------------------------------------------------------------
response_t mixin::request(aid_t recver, message const& m)
{
  response_t res(base_type::new_request(), get_aid());
  snd_.post(
    boost::bind(
      &base_type::pri_request, this,
      res, recver, m, base_type::sync
      )
    );
  return res;
}
///----------------------------------------------------------------------------
response_t mixin::request(svcid_t recver, message const& m)
{
  response_t res(base_type::new_request(), get_aid());
  snd_.post(
    boost::bind(
      &base_type::pri_request_svc, this,
      res, recver, m, base_type::sync
      )
    );
  return res;
}
///----------------------------------------------------------------------------
void mixin::reply(aid_t recver, message const& m)
{
  snd_.post(
    boost::bind(
      &base_type::pri_reply, this, recver, m, base_type::sync
      )
    );
}
///----------------------------------------------------------------------------
void mixin::link(aid_t target)
{
  snd_.post(
    boost::bind(
      &base_type::pri_link, this, target, base_type::sync
      )
    );
}
///----------------------------------------------------------------------------
void mixin::monitor(aid_t target)
{
  snd_.post(
    boost::bind(
      &base_type::pri_monitor, this, target, base_type::sync
      )
    );
}
///----------------------------------------------------------------------------
aid_t mixin::recv(message& msg, match const& mach)
{
  recv_promise_t p;
  recv_future_t f = p.get_future();

  snd_.post(
    boost::bind(
      &mixin::try_recv, this,
      boost::ref(p), boost::cref(mach)
      )
    );

  aid_t sender;
  recv_optional_t opt = f.get();
  if (opt)
  {
    recv_optional_t::reference_type rcv = boost::get(opt);
    if (aid_t* aid = boost::get<aid_t>(&rcv.first))
    {
      sender = *aid;
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&rcv.first))
    {
      sender = req->get_aid();
      msg.req_ = *req;
    }
    else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv.first))
    {
      sender = ex->get_aid();
    }
    msg = rcv.second;
  }
  return sender;
}
///----------------------------------------------------------------------------
aid_t mixin::recv(response_t res, message& msg, duration_t tmo)
{
  res_promise_t p;
  res_future_t f = p.get_future();

  snd_.post(
    boost::bind(
      &mixin::try_response, this,
      boost::ref(p), res, tmo
      )
    );

  aid_t sender;
  res_optional_t opt = f.get();
  if (opt)
  {
    res_optional_t::reference_type res_pr = boost::get(opt);
    res = res_pr.first;
    sender = res.get_aid();
    msg = res_pr.second;
  }
  return sender;
}
///----------------------------------------------------------------------------
void mixin::wait(duration_t dur)
{
  boost::this_thread::sleep_for(dur);
}
///----------------------------------------------------------------------------
void mixin::on_recv(detail::pack& pk, base_type::send_hint hint)
{
  if (hint == base_type::sync)
  {
    snd_.dispatch(
      boost::bind(
        &mixin::handle_recv, this, pk
        )
      );
  }
  else
  {
    snd_.post(
      boost::bind(
        &mixin::handle_recv, this, pk
        )
      );
  }
}
///----------------------------------------------------------------------------
sid_t mixin::spawn(match_t func, match_t ctxid, std::size_t stack_size)
{
  sid_t sid = base_type::new_request();
  snd_.post(
    boost::bind(
      &base_type::pri_spawn, this,
      sid, func, ctxid, stack_size, base_type::sync
      )
    );
  return sid;
}
///----------------------------------------------------------------------------
void mixin::try_recv(recv_promise_t& p, match const& mach)
{
  std::pair<detail::recv_t, message> rcv;
  if (!mb_.pop(rcv.first, rcv.second, mach.match_list_))
  {
    duration_t tmo = mach.timeout_;
    if (tmo > zero)
    {
      if (tmo < infin)
      {
        start_recv_timer(tmo, p);
      }
      recv_p_ = &p;
      curr_match_ = mach;
      return;
    }
  }

  p.set_value(rcv);
}
///----------------------------------------------------------------------------
void mixin::try_response(res_promise_t& p, response_t res, duration_t tmo)
{
  std::pair<response_t, message> res_pr;
  res_pr.first = res;
  if (!mb_.pop(res_pr.first, res_pr.second))
  {
    if (tmo > zero)
    {
      if (tmo < infin)
      {
        start_recv_timer(tmo, p);
      }
      res_p_ = &p;
      recving_res_ = res;
      return;
    }
  }

  p.set_value(res_pr);
}
///----------------------------------------------------------------------------
void mixin::start_recv_timer(duration_t dur, recv_promise_t& p)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &mixin::handle_recv_timeout, this,
        boost::asio::placeholders::error, boost::ref(p), ++tmr_sid_
        )
      )
    );
}
///----------------------------------------------------------------------------
void mixin::start_recv_timer(duration_t dur, res_promise_t& p)
{
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd_.wrap(
      boost::bind(
        &mixin::handle_res_timeout, this,
        boost::asio::placeholders::error, boost::ref(p), ++tmr_sid_
        )
      )
    );
}
///----------------------------------------------------------------------------
void mixin::handle_recv_timeout(
  errcode_t const& ec, recv_promise_t& p, std::size_t tmr_sid
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    /// timed out
    BOOST_ASSERT(&p == recv_p_);
    recv_p_ = 0;
    curr_match_.clear();
    std::pair<detail::recv_t, message> rcv;
    p.set_value(rcv);
  }
}
///----------------------------------------------------------------------------
void mixin::handle_res_timeout(
  errcode_t const& ec, res_promise_t& p, std::size_t tmr_sid
  )
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    /// timed out
    BOOST_ASSERT(&p == res_p_);
    res_p_ = 0;
    recving_res_ = response_t();
    std::pair<response_t, message> res_pr;
    p.set_value(res_pr);
  }
}
///----------------------------------------------------------------------------
void mixin::handle_recv(detail::pack& pk)
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

    detail::recv_t rcv;
    message msg;

    if (
      (recv_p_ && !is_response) ||
      (res_p_ && is_response)
      )
    {
      if (recv_p_ && !is_response)
      {
        bool ret = mb_.pop(rcv, msg, curr_match_.match_list_);
        if (!ret)
        {
          return;
        }
        recv_p_->set_value(std::make_pair(rcv, msg));
        recv_p_ = 0;
        curr_match_.clear();
      }

      if (res_p_ && is_response)
      {
        BOOST_ASSERT(recving_res_.valid());
        bool ret = mb_.pop(recving_res_, msg);
        if (!ret)
        {
          return;
        }
        res_p_->set_value(std::make_pair(recving_res_, msg));
        res_p_ = 0;
        recving_res_ = response_t();
      }

      ++tmr_sid_;
      errcode_t ec;
      tmr_.cancel(ec);
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
