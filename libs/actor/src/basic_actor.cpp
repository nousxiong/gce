///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/basic_actor.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/detail/scope.hpp>
#include <boost/variant/get.hpp>
#include <boost/foreach.hpp>

namespace gce
{
///----------------------------------------------------------------------------
basic_actor::basic_actor(std::size_t cache_match_size, timestamp_t const timestamp)
  : owner_(0)
  , user_(0)
  , mb_(cache_match_size)
  , req_id_(0)
  , timestamp_(timestamp)
{
  aid_ = aid_t(ctxid_nil, timestamp, this, 0);
}
///----------------------------------------------------------------------------
basic_actor::~basic_actor()
{
}
///----------------------------------------------------------------------------
void basic_actor::send(aid_t recver, message const& m)
{
  detail::pack* pk = alloc_pack(user_);
  pk->tag_ = get_aid();
  pk->recver_ = recver;
  pk->msg_ = m;

  recver.get_actor_ptr(
    user_->get_ctxid(),
    user_->get_context().get_timestamp()
    )->on_recv(pk);
}
///----------------------------------------------------------------------------
void basic_actor::relay(aid_t des, message& m)
{
  detail::pack* pk = alloc_pack(user_);
  if (m.req_.valid())
  {
    pk->tag_ = m.req_;
    m.req_ = detail::request_t();
  }
  else
  {
    pk->tag_ = get_aid();
  }
  pk->recver_ = des;
  pk->msg_ = m;

  des.get_actor_ptr(
    user_->get_ctxid(),
    user_->get_context().get_timestamp()
    )->on_recv(pk);
}
///----------------------------------------------------------------------------
response_t basic_actor::request(aid_t target, message const& m)
{
  aid_t sender = get_aid();
  response_t res(new_request(), sender);
  detail::request_t req(res.get_id(), sender);

  detail::pack* pk = alloc_pack(user_);
  pk->tag_ = req;
  pk->recver_ = target;
  pk->msg_ = m;

  target.get_actor_ptr(
    user_->get_ctxid(),
    user_->get_context().get_timestamp()
    )->on_recv(pk);
  return res;
}
///----------------------------------------------------------------------------
void basic_actor::reply(aid_t recver, message const& m)
{
  basic_actor* a =
    recver.get_actor_ptr(
      user_->get_ctxid(),
      user_->get_context().get_timestamp()
      );
  detail::request_t req;
  detail::pack* pk = alloc_pack(user_);
  if (mb_.pop(recver, req))
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
void basic_actor::on_free()
{
  mb_.clear();
  link_list_.clear();
  monitor_list_.clear();
  dealloc_pack(owner_, pack_que_.pop_all_reverse());
}
///----------------------------------------------------------------------------
detail::pack* basic_actor::alloc_pack(detail::cache_pool* owner)
{
  detail::pack* pk = owner->get_pack();
  pk->owner_ = owner;
  return pk;
}
///----------------------------------------------------------------------------
void basic_actor::dealloc_pack(detail::cache_pool* owner, detail::pack* pk)
{
  while (pk)
  {
    detail::pack* next = detail::node_access::get_next(pk);
    detail::node_access::set_next(pk, (detail::pack*)0);
    owner->free_pack(pk->owner_, pk);
    pk = next;
  }
}
///----------------------------------------------------------------------------
void basic_actor::add_link(aid_t target)
{
  link_list_.insert(target);
}
///----------------------------------------------------------------------------
void basic_actor::link(detail::link_t l, detail::cache_pool* user)
{
  aid_t target = l.get_aid();
  if (l.get_type() == linked)
  {
    add_link(target);
  }
  else
  {
    monitor_list_.insert(target);
  }

  if (user)
  {
    detail::pack* pk = alloc_pack(user);
    pk->tag_ = detail::link_t(l.get_type(), get_aid());
    pk->recver_ = target;

    target.get_actor_ptr(
      user->get_ctxid(),
      user->get_context().get_timestamp()
      )->on_recv(pk);
  }
}
///----------------------------------------------------------------------------
void basic_actor::send_exit(
  exit_code_t ec, std::string const& exit_msg, detail::cache_pool* user
  )
{
  BOOST_FOREACH(aid_t aid, link_list_)
  {
    detail::pack* pk = alloc_pack(user);
    pk->tag_ = detail::exit_t(ec, get_aid());
    pk->recver_ = aid;
    pk->msg_ = message(exit);
    pk->msg_ << ec << exit_msg;

    aid.get_actor_ptr(
      user->get_ctxid(),
      user->get_context().get_timestamp()
      )->on_recv(pk);
  }
}
///----------------------------------------------------------------------------
void basic_actor::remove_link(aid_t aid)
{
  link_list_.erase(aid);
  monitor_list_.erase(aid);
}
///----------------------------------------------------------------------------
void basic_actor::send_already_exited(
  aid_t recver, aid_t sender, detail::cache_pool* user
  )
{
  message m(exit);
  std::string exit_msg("already exited");
  m << exit_already << exit_msg;

  detail::pack* ret = alloc_pack(user);
  ret->tag_ = sender;
  ret->recver_ = recver;
  ret->msg_ = m;
  ret->is_err_ret_ = true;

  recver.get_actor_ptr(
    user->get_ctxid(),
    user->get_context().get_timestamp()
    )->on_recv(ret);
}
///----------------------------------------------------------------------------
void basic_actor::send_already_exited(
  aid_t recver, response_t res, detail::cache_pool* user
  )
{
  message m(exit);
  std::string exit_msg("already exited");
  m << exit_already << exit_msg;

  detail::pack* ret = alloc_pack(user);
  ret->tag_ = res;
  ret->recver_ = recver;
  ret->msg_ = m;
  ret->is_err_ret_ = true;

  recver.get_actor_ptr(
    user->get_ctxid(),
    user->get_context().get_timestamp()
    )->on_recv(ret);
}
///----------------------------------------------------------------------------
}
