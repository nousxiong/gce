///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/detail/cache_aligned_new.hpp>
#include <gce/detail/scope.hpp>
#include <boost/variant/get.hpp>
#include <boost/foreach.hpp>

namespace gce
{
///----------------------------------------------------------------------------
basic_actor::basic_actor(std::size_t cache_match_size)
  : owner_(0)
  , mb_(GCE_CACHE_ALIGNED_NEW(detail::mailbox)(cache_match_size), detail::cache_aligned_deleter())
  , pack_que_(GCE_CACHE_ALIGNED_NEW(detail::pack_queue_t), detail::cache_aligned_deleter())
  , req_id_(0)
{
  aid_ = aid_t(this, 0);
}
///----------------------------------------------------------------------------
basic_actor::~basic_actor()
{
}
///----------------------------------------------------------------------------
void basic_actor::on_free()
{
  mb_->clear();
  link_list_.clear();
  monitor_list_.clear();
  dealloc_pack(owner_.get(), pack_que_->pop_all_reverse());
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
void basic_actor::move_pack(detail::cache_pool* user)
{
  detail::pack* pk = pack_que_->pop_all();
  if (pk)
  {
    detail::scope scp(boost::bind(&basic_actor::dealloc_pack, user, pk));
    while (pk)
    {
      detail::pack* next = detail::node_access::get_next(pk);
      if (check(pk->recver_))
      {
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
          remove_link(ex->get_aid());
        }
        else if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
        {
          add_link(link->get_aid());
          return;
        }
        else if (response_t* res = boost::get<response_t>(&pk->tag_))
        {
          mb_->push(*res, pk->msg_);
        }
      }
      else
      {
        if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
        {
          /// send actor exit msg
          detail::pack* pk = alloc_pack(user);
          pk->tag_ = get_aid();
          pk->recver_ = link->get_aid();
          pk->msg_ = message(exit);
          std::string exit_msg("already exited");
          pk->msg_ << exit_already << exit_msg;

          basic_actor* a = pk->recver_.get_actor_ptr();
          a->on_recv(pk);
        }
      }
      pk = next;
    }
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

    basic_actor* a = target.get_actor_ptr();
    a->on_recv(pk);
  }
}
///----------------------------------------------------------------------------
void basic_actor::send_exit(exit_code_t ec, std::string const& exit_msg, detail::cache_pool* user)
{
  BOOST_FOREACH(aid_t aid, link_list_)
  {
    detail::pack* pk = alloc_pack(user);
    pk->tag_ = detail::exit_t(ec, get_aid());
    pk->recver_ = aid;
    pk->msg_ = message(exit);
    pk->msg_ << ec << exit_msg;

    basic_actor* a = aid.get_actor_ptr();
    a->on_recv(pk);
  }
}
///----------------------------------------------------------------------------
void basic_actor::remove_link(aid_t aid)
{
  link_list_.erase(aid);
  monitor_list_.erase(aid);
}
///----------------------------------------------------------------------------
}
