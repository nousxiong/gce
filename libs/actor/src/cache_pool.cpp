///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/detail/socket.hpp>
#include <gce/actor/detail/acceptor.hpp>
#include <boost/foreach.hpp>

namespace gce
{
namespace detail
{
///------------------------------------------------------------------------------
cache_pool::cache_pool(thread& thr, attributes const& attrs)
  : thr_(&thr)
  , actor_pool_(
      this, thr_,
      size_nil,
      attrs.actor_pool_reserve_size_
      )
  , socket_pool_(
      this, thr_,
      size_nil,
      attrs.socket_pool_reserve_size_
      )
  , acceptor_pool_(
      this, thr_,
      size_nil,
      attrs.acceptor_pool_reserve_size_
      )
  , pack_pool_(
      this,
      attrs.pack_pool_cache_size_,
      attrs.pack_pool_reserve_size_
      )
  , stopped_(false)
{
}
///------------------------------------------------------------------------------
cache_pool::~cache_pool()
{
  free_object();
}
///------------------------------------------------------------------------------
actor* cache_pool::get_actor()
{
  return actor_pool_.get();
}
///------------------------------------------------------------------------------
socket* cache_pool::get_socket()
{
  return socket_pool_.get();
}
///------------------------------------------------------------------------------
acceptor* cache_pool::get_acceptor()
{
  return acceptor_pool_.get();
}
///------------------------------------------------------------------------------
pack* cache_pool::get_pack()
{
  pack* pk = pack_pool_.get();
  pk->thr_ = thr_;
  return pk;
}
///------------------------------------------------------------------------------
void cache_pool::free_actor(actor* a)
{
  actor_pool_.free(a);
}
///------------------------------------------------------------------------------
void cache_pool::free_socket(socket* skt)
{
  socket_pool_.free(skt);
}
///------------------------------------------------------------------------------
void cache_pool::free_acceptor(acceptor* acpr)
{
  acceptor_pool_.free(acpr);
}
///------------------------------------------------------------------------------
void cache_pool::free_pack(pack* pk)
{
  pack_free_queue_.push(pk);
}
///------------------------------------------------------------------------------
void cache_pool::free_object()
{
  free_object<pack>(pack_pool_, pack_free_queue_);
}
///------------------------------------------------------------------------------
void cache_pool::register_service(match_t name, aid_t svc)
{
  service_list_.insert(std::make_pair(name, svc));
}
///------------------------------------------------------------------------------
aid_t cache_pool::find_service(match_t name)
{
  aid_t svc;
  service_list_t::iterator itr(service_list_.find(name));
  if (itr != service_list_.end())
  {
    svc = itr->second;
  }
  return svc;
}
///------------------------------------------------------------------------------
void cache_pool::deregister_service(match_t name, aid_t svc)
{
  service_list_t::iterator itr(service_list_.find(name));
  if (itr != service_list_.end() && itr->second == svc)
  {
    service_list_.erase(itr);
  }
}
///------------------------------------------------------------------------------
void cache_pool::register_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  if (ctxid_pr.second == socket_router)
  {
    std::pair<conn_list_t::iterator, bool> pr =
      router_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
    skt_list_t& skt_list = pr.first->second.skt_list_;
    skt_list.insert(skt);
    if (skt_list.size() == 1)
    {
      pr.first->second.curr_skt_ = skt_list.begin();
    }

    if (router_list_.size() == 1)
    {
      curr_router_list_ = router_list_.begin();
    }
  }
  else if (ctxid_pr.second == socket_joint)
  {
    std::pair<conn_list_t::iterator, bool> pr =
      joint_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
    skt_list_t& skt_list = pr.first->second.skt_list_;
    skt_list.insert(skt);
    if (skt_list.size() == 1)
    {
      pr.first->second.curr_skt_ = skt_list.begin();
    }

    if (joint_list_.size() == 1)
    {
      curr_joint_list_ = joint_list_.begin();
    }
  }
  else
  {
    std::pair<conn_list_t::iterator, bool> pr =
      conn_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
    skt_list_t& skt_list = pr.first->second.skt_list_;
    skt_list.insert(skt);
    if (skt_list.size() == 1)
    {
      pr.first->second.curr_skt_ = skt_list.begin();
    }

    if (conn_list_.size() == 1)
    {
      curr_socket_list_ = conn_list_.begin();
    }
  }
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_socket(ctxid_t ctxid, ctxid_t* target)
{
  aid_t skt = select_straight_socket(ctxid, target);
  if (!skt)
  {
    skt = select_router(target);
  }
  return skt;
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_straight_socket(ctxid_t ctxid, ctxid_t* target)
{
  aid_t skt;
  skt_list_t* skt_list = 0;
  skt_list_t::iterator* curr_skt = 0;

  if (ctxid != ctxid_nil)
  {
    conn_list_t::iterator itr(conn_list_.find(ctxid));
    if (itr != conn_list_.end())
    {
      skt_list = &itr->second.skt_list_;
      curr_skt = &itr->second.curr_skt_;
    }
  }
  else
  {
    if (!conn_list_.empty())
    {
      if (curr_socket_list_ != conn_list_.end())
      {
        skt_list = &curr_socket_list_->second.skt_list_;
        curr_skt = &curr_socket_list_->second.curr_skt_;
        if (target)
        {
          *target = curr_socket_list_->first;
        }
      }

      ++curr_socket_list_;
      if (curr_socket_list_ == conn_list_.end())
      {
        curr_socket_list_ = conn_list_.begin();
      }
    }
  }

  if (skt_list && !skt_list->empty())
  {
    BOOST_ASSERT(curr_skt);
    skt_list_t::iterator& itr = *curr_skt;
    if (itr != skt_list->end())
    {
      skt = *itr;
    }
    ++itr;
    if (itr == skt_list->end())
    {
      itr = skt_list->begin();
    }
  }

  return skt;
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_joint_socket(ctxid_t ctxid, ctxid_t* target)
{
  aid_t skt;
  skt_list_t* skt_list = 0;
  skt_list_t::iterator* curr_skt = 0;

  if (ctxid != ctxid_nil)
  {
    conn_list_t::iterator itr(joint_list_.find(ctxid));
    if (itr != joint_list_.end())
    {
      skt_list = &itr->second.skt_list_;
      curr_skt = &itr->second.curr_skt_;
    }
  }
  else
  {
    if (!joint_list_.empty())
    {
      if (curr_joint_list_ != joint_list_.end())
      {
        skt_list = &curr_joint_list_->second.skt_list_;
        curr_skt = &curr_joint_list_->second.curr_skt_;
        if (target)
        {
          *target = curr_joint_list_->first;
        }
      }

      ++curr_joint_list_;
      if (curr_joint_list_ == joint_list_.end())
      {
        curr_joint_list_ = joint_list_.begin();
      }
    }
  }

  if (skt_list && !skt_list->empty())
  {
    BOOST_ASSERT(curr_skt);
    skt_list_t::iterator& itr = *curr_skt;
    if (itr != skt_list->end())
    {
      skt = *itr;
    }
    ++itr;
    if (itr == skt_list->end())
    {
      itr = skt_list->begin();
    }
  }

  return skt;
}
///------------------------------------------------------------------------------
aid_t cache_pool::select_router(ctxid_t* target)
{
  aid_t skt;
  if (!router_list_.empty())
  {
    if (curr_router_list_ != router_list_.end())
    {
      skt_list_t& skt_list = curr_router_list_->second.skt_list_;
      skt_list_t::iterator& curr_skt = curr_router_list_->second.curr_skt_;
      if (target)
      {
        *target = curr_router_list_->first;
      }

      if (!skt_list.empty())
      {
        if (curr_skt != skt_list.end())
        {
          skt = *curr_skt;
        }
        ++curr_skt;
        if (curr_skt == skt_list.end())
        {
          curr_skt = skt_list.begin();
        }
      }
    }

    ++curr_router_list_;
    if (curr_router_list_ == router_list_.end())
    {
      curr_router_list_ = router_list_.begin();
    }
  }
  return skt;
}
///------------------------------------------------------------------------------
void cache_pool::deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  if (ctxid_pr.second == socket_router)
  {
    conn_list_t::iterator itr(router_list_.find(ctxid_pr.first));
    if (itr != router_list_.end())
    {
      socket_list& skt_list = itr->second;
      skt_list.skt_list_.erase(skt);
      skt_list.curr_skt_ = skt_list.skt_list_.begin();
    }
  }
  else if (ctxid_pr.second == socket_joint)
  {
    conn_list_t::iterator itr(joint_list_.find(ctxid_pr.first));
    if (itr != joint_list_.end())
    {
      socket_list& skt_list = itr->second;
      skt_list.skt_list_.erase(skt);
      skt_list.curr_skt_ = skt_list.skt_list_.begin();
    }
  }
  else
  {
    conn_list_t::iterator itr(conn_list_.find(ctxid_pr.first));
    if (itr != conn_list_.end())
    {
      socket_list& skt_list = itr->second;
      skt_list.skt_list_.erase(skt);
      skt_list.curr_skt_ = skt_list.skt_list_.begin();
    }
  }
}
///------------------------------------------------------------------------------
void cache_pool::add_socket(socket* s)
{
  socket_list_.insert(s);
}
///------------------------------------------------------------------------------
void cache_pool::add_acceptor(acceptor* a)
{
  acceptor_list_.insert(a);
}
///------------------------------------------------------------------------------
void cache_pool::remove_socket(socket* s)
{
  socket_list_.erase(s);
}
///------------------------------------------------------------------------------
void cache_pool::remove_acceptor(acceptor* a)
{
  acceptor_list_.erase(a);
}
///------------------------------------------------------------------------------
void cache_pool::stop()
{
  stopped_ = true;

  BOOST_FOREACH(socket* s, socket_list_)
  {
    s->stop();
  }
  socket_list_.clear();

  BOOST_FOREACH(acceptor* a, acceptor_list_)
  {
    a->stop();
  }
  acceptor_list_.clear();
}
///------------------------------------------------------------------------------
template <typename T, typename Pool, typename FreeQueue>
void cache_pool::free_object(Pool& pool, FreeQueue& free_que)
{
  T* o = free_que.pop_all_reverse();
  while (o)
  {
    T* next = node_access::get_next(o);
    node_access::set_next(o, (T*)0);
    pool.free(o);
    o = next;
  }
}
///------------------------------------------------------------------------------
}
}
