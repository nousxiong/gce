///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/thin.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
thin::thin(detail::thin_attrs attrs)
  : basic_actor(attrs.cache_match_size_)
  , user_(0)
  , f_(GCE_CACHE_ALIGNED_NEW(func_t), detail::cache_aligned_deleter())
  , recving_(false)
  , responsing_(false)
  , sender_(0)
  , msg_(0)
  , tmr_(*attrs.ctx_->get_io_service())
  , tmr_sid_(0)
{
}
///----------------------------------------------------------------------------
thin::~thin()
{
}
///----------------------------------------------------------------------------
void thin::recv(aid_t& sender, message& msg, match const& mach)
{
  if (!mb_->pop(rcv_, msg, mach.match_list_))
  {
    duration_t tmo = mach.timeout_;
    curr_mach_ = mach;
    if (tmo < infin)
    {
      start_recv_timer(tmo);
    }
    sender_ = &sender;
    msg_ = &msg;
    recving_ = true;
    return;
  }

  if (aid_t* aid = boost::get<aid_t>(&rcv_))
  {
    sender = *aid;
  }
  else if (detail::request_t* req = boost::get<detail::request_t>(&rcv_))
  {
    sender = req->get_aid();
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv_))
  {
    sender = ex->get_aid();
  }

  strand_t* snd = user_->get_strand();
  snd->post(boost::bind(&thin::end_recv, this));
}
///----------------------------------------------------------------------------
aid_t thin::recv(message& msg, match_list_t const& match_list)
{
  aid_t sender;
  if (!mb_->pop(rcv_, msg, match_list))
  {
    return sender;
  }

  if (aid_t* aid = boost::get<aid_t>(&rcv_))
  {
    sender = *aid;
  }
  else if (detail::request_t* req = boost::get<detail::request_t>(&rcv_))
  {
    sender = req->get_aid();
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv_))
  {
    sender = ex->get_aid();
  }

  return sender;
}
///----------------------------------------------------------------------------
void thin::send(aid_t recver, message const& m)
{
  detail::pack* pk = base_type::alloc_pack(user_.get());
  pk->tag_ = get_aid();
  pk->recver_ = recver;
  pk->msg_ = m;

  basic_actor* a = recver.get_actor_ptr();
  a->on_recv(pk);
}
///----------------------------------------------------------------------------
response_t thin::request(aid_t target, message const& m)
{
  aid_t sender = get_aid();
  response_t res(new_request(), sender);
  detail::request_t req(res.get_id(), sender);

  detail::pack* pk = base_type::alloc_pack(user_.get());
  pk->tag_ = req;
  pk->recver_ = target;
  pk->msg_ = m;

  basic_actor* a = target.get_actor_ptr();
  a->on_recv(pk);
  return res;
}
///----------------------------------------------------------------------------
void thin::reply(aid_t recver, message const& m)
{
  basic_actor* a = recver.get_actor_ptr();
  detail::request_t req;
  detail::pack* pk = base_type::alloc_pack(user_.get());
  if (mb_->pop(recver, req))
  {
    response_t res(req.get_id(), get_aid());
    pk->tag_ = res;
    pk->recver_ = recver;
    pk->msg_ = m;
  }
  else
  {
    pk->tag_ = get_aid();
    pk->recver_ = recver;
    pk->msg_ = m;
  }
  a->on_recv(pk);
}
///----------------------------------------------------------------------------
void thin::recv(response_t res, aid_t& sender, message& msg, duration_t tmo)
{
  res_ = res;
  if (!mb_->pop(res_, msg))
  {
    if (tmo < infin)
    {
      start_recv_timer(tmo);
    }
    sender_ = &sender;
    msg_ = &msg;
    responsing_ = true;
    return;
  }

  sender = res_.get_aid();
  strand_t* snd = user_->get_strand();
  snd->post(boost::bind(&thin::end_response, this));
}
///----------------------------------------------------------------------------
aid_t thin::recv(response_t res, message& msg)
{
  aid_t sender;
  if (!mb_->pop(res, msg))
  {
    return sender;
  }

  sender = res.get_aid();
  return sender;
}
///----------------------------------------------------------------------------
void thin::wait(duration_t dur)
{
  if (dur < infin)
  {
    start_recv_timer(dur);
  }
}
///----------------------------------------------------------------------------
void thin::link(aid_t target)
{
  base_type::link(detail::link_t(linked, target), user_.get());
}
///----------------------------------------------------------------------------
void thin::monitor(aid_t target)
{
  base_type::link(detail::link_t(monitored, target), user_.get());
}
///----------------------------------------------------------------------------
detail::cache_pool* thin::get_cache_pool()
{
  return user_.get();
}
///----------------------------------------------------------------------------
void thin::start()
{
  strand_t* snd = user_->get_strand();
  snd->dispatch(boost::bind(&thin::begin_run, this));
}
///----------------------------------------------------------------------------
void thin::init(detail::cache_pool* user, detail::cache_pool* owner, thin_func_t const& f, aid_t link_tgt)
{
  user_ = user;
  owner_ = owner;
  *f_ = f;
  base_type::update_aid();

  if (link_tgt)
  {
    base_type::add_link(link_tgt);
  }
}
///----------------------------------------------------------------------------
void thin::on_free()
{
  base_type::on_free();

  f_->clear();
  coro_ = detail::coro_t();
  curr_mach_.clear();

  recving_ = false;
  responsing_ = false;
  sender_ = 0;
  msg_ = 0;
  rcv_ = detail::recv_t();
  res_ = response_t();
}
///----------------------------------------------------------------------------
void thin::on_recv(detail::pack* pk)
{
  strand_t* snd = user_->get_strand();
  snd->dispatch(
    boost::bind(
      &thin::handle_recv, this, pk
      )
    );
}
///----------------------------------------------------------------------------
void thin::run()
{
  try
  {
    (*f_)(*this);
    if (coro_.is_complete())
    {
      free_self(exit_normal, "exit normal");
    }
  }
  catch (std::exception& ex)
  {
    free_self(exit_except, ex.what());
  }
  catch (...)
  {
    free_self(exit_unknown, "unexpected exception");
  }
}
///----------------------------------------------------------------------------
void thin::handle_recv(detail::pack* pk)
{
  detail::scope scp(boost::bind(&basic_actor::dealloc_pack, user_.get(), pk));
  if (check(pk->recver_))
  {
    bool is_response = false;
    if (aid_t* aid = boost::get<aid_t>(&pk->tag_))
    {
      mb_->push(*aid, pk->msg_);
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
    {
      mb_->push(*req, pk->msg_);
    }
    else if (detail::exit_t* ex = boost::get<detail::exit_t>(&pk->tag_))
    {
      mb_->push(*ex, pk->msg_);
      base_type::remove_link(ex->get_aid());
    }
    else if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
    {
      add_link(link->get_aid());
      return;
    }
    else if (response_t* res = boost::get<response_t>(&pk->tag_))
    {
      is_response = true;
      mb_->push(*res, pk->msg_);
    }

    bool have_msg = false;
    if (is_response)
    {
      if (responsing_)
      {
        BOOST_ASSERT(sender_);
        BOOST_ASSERT(msg_);
        BOOST_ASSERT(res_.valid());
        have_msg = mb_->pop(res_, *msg_);
        if (have_msg)
        {
          *sender_ = res_.get_aid();
        }
      }
    }
    else
    {
      if (recving_)
      {
        BOOST_ASSERT(sender_);
        BOOST_ASSERT(msg_);
        have_msg = mb_->pop(rcv_, *msg_, curr_mach_.match_list_);
        if (have_msg)
        {
          curr_mach_.clear();
          if (aid_t* aid = boost::get<aid_t>(&rcv_))
          {
            *sender_ = *aid;
          }
          else if (detail::request_t* req = boost::get<detail::request_t>(&rcv_))
          {
            *sender_ = req->get_aid();
          }
        }
      }
    }

    if (have_msg)
    {
      ++tmr_sid_;
      errcode_t ec;
      tmr_.cancel(ec);

      if (responsing_)
      {
        end_response();
      }
      else
      {
        end_recv();
      }
    }
  }
  else if (!pk->is_err_ret_)
  {
    if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
    {
      /// send actor exit msg
      base_type::send_already_exited(link->get_aid(), pk->recver_, user_.get());
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
    {
      /// reply actor exit msg
      response_t res(req->get_id(), pk->recver_);
      base_type::send_already_exited(req->get_aid(), res, user_.get());
    }
  }
}
///----------------------------------------------------------------------------
void thin::begin_run()
{
  run();
}
///----------------------------------------------------------------------------
void thin::free_self(exit_code_t ec, std::string const& exit_msg)
{
  base_type::send_exit(ec, exit_msg, user_.get());
  base_type::update_aid();
  user_->free_thin(owner_.get(), this);
}
///----------------------------------------------------------------------------
void thin::start_recv_timer(duration_t dur)
{
  strand_t* snd = user_->get_strand();
  tmr_.expires_from_now(dur);
  tmr_.async_wait(
    snd->wrap(
      boost::bind(
        &thin::handle_recv_timeout, this,
        boost::asio::placeholders::error, ++tmr_sid_
        )
      )
    );
}
///----------------------------------------------------------------------------
void thin::handle_recv_timeout(errcode_t const& ec, std::size_t tmr_sid)
{
  if (!ec && tmr_sid == tmr_sid_)
  {
    if (recving_)
    {
      end_recv();
    }
    else if (responsing_)
    {
      end_response();
    }
    else
    {
      end_wait();
    }
  }
}
///----------------------------------------------------------------------------
void thin::end_recv()
{
  rcv_ = detail::recv_t();
  sender_ = 0;
  msg_= 0;
  recving_ = false;
  run();
}
///----------------------------------------------------------------------------
void thin::end_response()
{
  res_ = response_t();
  sender_ = 0;
  msg_= 0;
  responsing_ = false;
  run();
}
///----------------------------------------------------------------------------
void thin::end_wait()
{
  run();
}
///----------------------------------------------------------------------------
}
