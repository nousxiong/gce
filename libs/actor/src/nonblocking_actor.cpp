///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/nonblocking_actor.hpp>
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
nonblocking_actor::nonblocking_actor(context& ctx, std::size_t index)
  : base_type(&ctx, ctx.select_cache_pool(), index)
  , cache_queue_list_(ctx_->get_cache_queue_size())
  , pack_queue_(1024)
  , cac_pool_(ctx, index, true)
{
  base_type::update_aid();
  user_ = &cac_pool_;
}
///----------------------------------------------------------------------------
nonblocking_actor::~nonblocking_actor()
{
}
///----------------------------------------------------------------------------
aid_t nonblocking_actor::recv(message& msg, match_list_t const& match_list)
{
  aid_t sender;
  detail::recv_t rcv;

  move_pack();
  if (!mb_.pop(rcv, msg, match_list))
  {
    return sender;
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
aid_t nonblocking_actor::recv(response_t res, message& msg)
{
  aid_t sender;

  move_pack();
  if (!mb_.pop(res, msg))
  {
    return sender;
  }

  sender = res.get_aid();
  return sender;
}
///----------------------------------------------------------------------------
void nonblocking_actor::on_recv(detail::pack& pk, base_type::send_hint)
{
  BOOST_ASSERT(pk.cache_queue_index_ != size_nil);

  cache_queue& cac_que = cache_queue_list_[pk.cache_queue_index_];
  boost::uint64_t garbage_tail = 
    cac_que.garbage_tail_.load(boost::memory_order_relaxed);

  if (garbage_tail != u64_nil && garbage_tail >= cac_que.head_)
  {
    std::size_t size = garbage_tail - cac_que.head_ + 1;
    BOOST_ASSERT(cac_que.que_.size() >= size);
    for (std::size_t i=0; i<size; ++i)
    {
      cac_que.que_.pop_front();
    }

    if (cac_que.que_.empty())
    {
      cac_que.head_ = cac_que.index_base_;
    }
    else
    {
      cac_que.head_ = cac_que.que_.front().cache_index_;
    }
  }

  pk.cache_index_ = cac_que.index_base_++;
  cac_que.que_.push_back(pk);
  pack_queue_.push(&cac_que.que_.back());
}
///------------------------------------------------------------------------------
void nonblocking_actor::register_service(match_t name, aid_t svc, std::size_t cache_queue_index)
{
  detail::pack pk;
  pk.cache_queue_index_ = cache_queue_index;
  aid_t self = get_aid();
  pk.tag_ = self;
  pk.recver_ = self;

  message m(detail::msg_reg_svc);
  m << name << svc;
  pk.msg_ = m;
  on_recv(pk, base_type::sync);
}
///------------------------------------------------------------------------------
void nonblocking_actor::deregister_service(match_t name, aid_t svc, std::size_t cache_queue_index)
{
  detail::pack pk;
  pk.cache_queue_index_ = cache_queue_index;
  aid_t self = get_aid();
  pk.tag_ = self;
  pk.recver_ = self;

  message m(detail::msg_dereg_svc);
  m << name << svc;
  pk.msg_ = m;
  on_recv(pk, base_type::sync);
}
///------------------------------------------------------------------------------
void nonblocking_actor::register_socket(ctxid_pair_t ctxid_pr, aid_t skt, std::size_t cache_queue_index)
{
  detail::pack pk;
  pk.cache_queue_index_ = cache_queue_index;
  aid_t self = get_aid();
  pk.tag_ = self;
  pk.recver_ = self;

  message m(detail::msg_reg_skt);
  m << ctxid_pr << skt;
  pk.msg_ = m;
  on_recv(pk, base_type::sync);
}
///------------------------------------------------------------------------------
void nonblocking_actor::deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt, std::size_t cache_queue_index)
{
  detail::pack pk;
  pk.cache_queue_index_ = cache_queue_index;
  aid_t self = get_aid();
  pk.tag_ = self;
  pk.recver_ = self;

  message m(detail::msg_dereg_skt);
  m << ctxid_pr << skt;
  pk.msg_ = m;
  on_recv(pk, base_type::sync);
}
///----------------------------------------------------------------------------
void nonblocking_actor::release_pack()
{
  for (std::size_t i=0, size=gc_.size(); i<size; ++i)
  {
    detail::pack* pk = gc_[i];
    if (pk)
    {
      cache_queue_list_[i].garbage_tail_ = pk->cache_index_;
    }
  }
}
///----------------------------------------------------------------------------
void nonblocking_actor::move_pack()
{
  gc_.clear();
  gc_.resize(ctx_->get_cache_queue_size(), 0);
  detail::scope scp(boost::bind(&nonblocking_actor::release_pack, this));
  detail::pack* pk = 0;
  while (pack_queue_.pop(pk))
  {
    gc_[pk->cache_queue_index_] = pk;
    handle_recv(*pk);
  }
}
///----------------------------------------------------------------------------
void nonblocking_actor::handle_recv(detail::pack& pk)
{
  if (check(pk.recver_, ctxid_, timestamp_))
  {
    bool is_response = false;
    if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
    {
      match_t type = pk.msg_.get_type();
      if (type == detail::msg_reg_skt)
      {
        ctxid_pair_t ctxid_pr;
        aid_t skt;
        pk.msg_ >> ctxid_pr >> skt;
        user_->register_socket(ctxid_pr, skt);
      }
      else if (type == detail::msg_dereg_skt)
      {
        ctxid_pair_t ctxid_pr;
        aid_t skt;
        pk.msg_ >> ctxid_pr >> skt;
        user_->deregister_socket(ctxid_pr, skt);
      }
      else if (type == detail::msg_reg_svc)
      {
        match_t name;
        aid_t svc;
        pk.msg_ >> name >> svc;
        user_->register_service(name, svc);
      }
      else if (type == detail::msg_dereg_svc)
      {
        match_t name;
        aid_t svc;
        pk.msg_ >> name >> svc;
        user_->deregister_service(name, svc);
      }
      else
      {
        mb_.push(*aid, pk.msg_);
      }
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
