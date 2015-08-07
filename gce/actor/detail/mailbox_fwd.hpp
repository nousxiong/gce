///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_MAILBOX_FWD_HPP
#define GCE_ACTOR_DETAIL_MAILBOX_FWD_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/detail/msg_pool.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/detail/exit.hpp>
#include <gce/detail/object_pool.hpp>
#include <boost/variant/variant.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>

namespace gce
{
namespace detail
{
typedef boost::variant<aid_t, request_t, exit_t> recv_t;
///----------------------------------------------------------------------------
/// recv_pair
///----------------------------------------------------------------------------
struct recv_pair
  : public linked_elem<recv_pair>
{
  void init(recv_t const& rcv, message* msg)
  {
    rcv_ = rcv;
    msg_ = msg;
  }

  void on_free()
  {
    msg_ = 0;
  }

  recv_t rcv_;
  message* msg_;

  boost::intrusive::list_member_hook<> hook_;
};

typedef object_pool<recv_pair> recv_pair_pool_t;

struct recv_pair_disposer
{
  explicit recv_pair_disposer(recv_pair_pool_t& pool, msg_pool_t* msg_pool = 0)
    : pool_(pool)
    , msg_pool_(msg_pool)
  {
  }

  void operator()(recv_pair* p)
  {
    if (msg_pool_ != 0)
    {
      msg_pool_->free(p->msg_);
    }
    pool_.free(p);
  }

  recv_pair_pool_t& pool_;
  msg_pool_t* msg_pool_;
};

typedef boost::intrusive::list<
  recv_pair, 
  boost::intrusive::member_hook<recv_pair, boost::intrusive::list_member_hook<>, &recv_pair::hook_> 
  > recv_queue_t;
///----------------------------------------------------------------------------
/// recv_itr
///----------------------------------------------------------------------------
struct recv_itr
  : public linked_elem<recv_itr>
{
  void init(recv_queue_t::iterator& itr)
  {
    itr_ = itr;
  }

  void on_free()
  {
    itr_ = recv_queue_t::iterator();
  }

  recv_queue_t::iterator itr_;

  boost::intrusive::list_member_hook<> hook_;
};

typedef object_pool<recv_itr> recv_itr_pool_t;

struct recv_itr_disposer
{
  explicit recv_itr_disposer(recv_itr_pool_t& pool)
    : pool_(pool)
  {
  }

  void operator()(recv_itr* p)
  {
    pool_.free(p);
  }

  recv_itr_pool_t& pool_;
};

typedef boost::intrusive::list<
  recv_itr, 
  boost::intrusive::member_hook<recv_itr, boost::intrusive::list_member_hook<>, &recv_itr::hook_> 
  > match_queue_t;
///----------------------------------------------------------------------------
/// match_queue
///----------------------------------------------------------------------------
struct match_queue
  : public linked_elem<match_queue>
{
  void init(match_t ty, recv_itr_pool_t& pool)
  {
    ty_ = ty;
    pool_ = &pool;
  }

  void on_free()
  {
    que_.clear_and_dispose(recv_itr_disposer(*pool_));
  }

  match_t ty_;
  match_queue_t que_;
  recv_itr_pool_t* pool_;

  boost::intrusive::set_member_hook<> hook_;
};

inline bool operator<(match_queue const& lhs, match_queue const& rhs)
{
  return lhs.ty_ < rhs.ty_;
}

//inline bool operator>(match_queue const& lhs, match_queue const& rhs)
//{
//  return rhs.ty_ < lhs.ty_;
//}

//inline bool operator==(match_queue const& lhs, match_queue const& rhs)
//{
//  return lhs.ty_ == rhs.ty_;
//}

typedef object_pool<match_queue> match_queue_pool_t;

struct match_queue_disposer
{
  explicit match_queue_disposer(match_queue_pool_t& pool)
    : pool_(pool)
  {
  }

  void operator()(match_queue* p)
  {
    pool_.free(p);
  }

  match_queue_pool_t& pool_;
};

typedef boost::intrusive::set<
  match_queue, 
  boost::intrusive::member_hook<match_queue, boost::intrusive::set_member_hook<>, &match_queue::hook_> 
  > match_queue_list_t;
///----------------------------------------------------------------------------
/// res_msg_pair
///----------------------------------------------------------------------------
struct res_msg_pair
  : public linked_elem<res_msg_pair>
{
  void init(sid_t sid, resp_t const& resp, message* msg)
  {
    sid_ = sid;
    resp_ = resp;
    msg_ = msg;
  }

  void on_free()
  {
    sid_ = sid_nil;
    msg_ = 0;
  }

  sid_t sid_;
  resp_t resp_;
  message* msg_;

  boost::intrusive::set_member_hook<> hook_;
};

inline bool operator<(res_msg_pair const& lhs, res_msg_pair const& rhs)
{
  return lhs.sid_ < rhs.sid_;
}

typedef object_pool<res_msg_pair> res_msg_pair_pool_t;

struct res_msg_pair_disposer
{
  explicit res_msg_pair_disposer(res_msg_pair_pool_t& pool, msg_pool_t* msg_pool = 0)
    : pool_(pool)
    , msg_pool_(msg_pool)
  {
  }

  void operator()(res_msg_pair* p)
  {
    if (msg_pool_ != 0)
    {
      msg_pool_->free(p->msg_);
    }
    pool_.free(p);
  }

  res_msg_pair_pool_t& pool_;
  msg_pool_t* msg_pool_;
};

typedef boost::intrusive::set<
  res_msg_pair, 
  boost::intrusive::member_hook<res_msg_pair, boost::intrusive::set_member_hook<>, &res_msg_pair::hook_> 
  > res_msg_pair_list_t;
///----------------------------------------------------------------------------
/// pools
///----------------------------------------------------------------------------
struct mailbox_pool_set
{
  mailbox_pool_set(
    size_t recv_reserve_size, size_t recv_grow_size, 
    size_t match_que_reserve_size, size_t match_que_grow_size, 
    size_t res_reserve_size, size_t res_grow_size
    )
    : recv_pair_(recv_reserve_size, recv_grow_size)
    , recv_itr_(recv_reserve_size, recv_grow_size)
    , match_que_(match_que_reserve_size, match_que_grow_size)
    , res_msg_pair_(res_reserve_size, res_grow_size)
  {
  }

  recv_pair_pool_t recv_pair_;
  recv_itr_pool_t recv_itr_;
  match_queue_pool_t match_que_;
  res_msg_pair_pool_t res_msg_pair_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_MAILBOX_FWD_HPP
