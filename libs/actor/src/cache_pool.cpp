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
#include <gce/actor/thin.hpp>
#include <gce/actor/detail/socket.hpp>
#include <gce/actor/detail/acceptor.hpp>
#include <boost/foreach.hpp>

namespace gce
{
namespace detail
{
///------------------------------------------------------------------------------
cache_pool::cache_pool(context& ctx, std::size_t id, attributes const& attrs, bool mixed)
  : ctx_(&ctx)
  , id_(id)
  , cache_num_(attrs.thread_num_*attrs.per_thread_cache_ + attrs.mixin_num_*attrs.per_mixin_cache_)
  , cache_match_size_(attrs.max_cache_match_size_)
  , mixed_(mixed)
  , snd_(GCE_CACHE_ALIGNED_NEW(strand_t)(*ctx.get_io_service()), cache_aligned_deleter())
  , gc_tmr_(GCE_CACHE_ALIGNED_NEW(boost::asio::system_timer)(*ctx.get_io_service()), cache_aligned_deleter())
  , actor_pool_(
      GCE_CACHE_ALIGNED_NEW(actor_pool_t)(
        this, detail::actor_attrs(&ctx, attrs.stack_scale_, cache_match_size_), size_nil,
        attrs.actor_pool_reserve_size_
        ),
      cache_aligned_deleter()
      )
  , thin_pool_(
      GCE_CACHE_ALIGNED_NEW(thin_pool_t)(
        this, detail::thin_attrs(&ctx, cache_match_size_), size_nil,
        attrs.thin_pool_reserve_size_
        ),
      cache_aligned_deleter()
      )
  , pack_pool_(
      GCE_CACHE_ALIGNED_NEW(pack_pool_t)(
        this,
        attrs.pack_pool_free_size_,
        attrs.pack_pool_reserve_size_
        ),
      cache_aligned_deleter()
      )
  , socket_pool_(
      GCE_CACHE_ALIGNED_NEW(socket_pool_t)(
        this, &ctx,
        attrs.socket_pool_free_size_,
        attrs.socket_pool_reserve_size_
        ),
      cache_aligned_deleter()
      )
  , acceptor_pool_(
      GCE_CACHE_ALIGNED_NEW(acceptor_pool_t)(
        this, &ctx,
        attrs.acceptor_pool_free_size_,
        attrs.acceptor_pool_reserve_size_
        ),
      cache_aligned_deleter()
      )
  , actor_free_queue_(GCE_CACHE_ALIGNED_NEW(actor_queue_t), cache_aligned_deleter())
  , thin_free_queue_(GCE_CACHE_ALIGNED_NEW(thin_queue_t), cache_aligned_deleter())
  , pack_free_queue_(GCE_CACHE_ALIGNED_NEW(pack_queue_t), cache_aligned_deleter())
  , socket_free_queue_(GCE_CACHE_ALIGNED_NEW(socket_queue_t), cache_aligned_deleter())
  , acceptor_free_queue_(GCE_CACHE_ALIGNED_NEW(acceptor_queue_t), cache_aligned_deleter())
  , actor_cache_list_(cache_num_)
  , thin_cache_list_(cache_num_)
  , pack_cache_list_(cache_num_)
  , socket_cache_list_(cache_num_)
  , acceptor_cache_list_(cache_num_)
{
  actor_cache_dirty_list_.reserve(cache_num_);
  thin_cache_dirty_list_.reserve(cache_num_);
  pack_cache_dirty_list_.reserve(cache_num_);
  socket_cache_dirty_list_.reserve(cache_num_);
  acceptor_cache_dirty_list_.reserve(cache_num_);
}
///------------------------------------------------------------------------------
cache_pool::~cache_pool()
{
  free_object();
}
///------------------------------------------------------------------------------
actor* cache_pool::get_actor()
{
  if (!mixed_ && !snd_->running_in_this_thread())
  {
    throw std::runtime_error("get_actor strand thread error");
  }

  return actor_pool_->get();
}
///------------------------------------------------------------------------------
thin* cache_pool::get_thin()
{
  if (!mixed_ && !snd_->running_in_this_thread())
  {
    throw std::runtime_error("get_thin strand thread error");
  }

  return thin_pool_->get();
}
///------------------------------------------------------------------------------
pack* cache_pool::get_pack()
{
  if (!mixed_ && !snd_->running_in_this_thread())
  {
    throw std::runtime_error("get_pack strand thread error");
  }

  return pack_pool_->get();
}
///------------------------------------------------------------------------------
socket* cache_pool::get_socket()
{
  if (!mixed_ && !snd_->running_in_this_thread())
  {
    throw std::runtime_error("get_socket strand thread error");
  }

  return socket_pool_->get();
}
///------------------------------------------------------------------------------
acceptor* cache_pool::get_acceptor()
{
  if (!mixed_ && !snd_->running_in_this_thread())
  {
    throw std::runtime_error("get_acceptor strand thread error");
  }

  return acceptor_pool_->get();
}
///------------------------------------------------------------------------------
void cache_pool::free_actor(cache_pool* owner, actor* a)
{
  free_object(
    owner, a, actor_cache_list_, actor_pool_.get(),
    owner->actor_free_queue_.get(), actor_cache_dirty_list_
    );
}
///------------------------------------------------------------------------------
void cache_pool::free_thin(cache_pool* owner, thin* t)
{
  free_object(
    owner, t, thin_cache_list_, thin_pool_.get(),
    owner->thin_free_queue_.get(), thin_cache_dirty_list_
    );
}
///------------------------------------------------------------------------------
void cache_pool::free_pack(cache_pool* owner, pack* pk)
{
  free_object(
    owner, pk, pack_cache_list_, pack_pool_.get(),
    owner->pack_free_queue_.get(), pack_cache_dirty_list_
    );
}
///------------------------------------------------------------------------------
void cache_pool::free_socket(cache_pool* owner, socket* skt)
{
  free_object(
    owner, skt, socket_cache_list_, socket_pool_.get(),
    owner->socket_free_queue_.get(), socket_cache_dirty_list_
    );
}
///------------------------------------------------------------------------------
void cache_pool::free_acceptor(cache_pool* owner, acceptor* acpr)
{
  free_object(
    owner, acpr, acceptor_cache_list_, acceptor_pool_.get(),
    owner->acceptor_free_queue_.get(), acceptor_cache_dirty_list_
    );
}
///------------------------------------------------------------------------------
template <typename T, typename Pool, typename FreeQueue, typename DirtyList>
void cache_pool::free_object(
  cache_pool* owner, T* o, std::vector<cache<T, FreeQueue> >& cache_list,
  Pool* pool, FreeQueue* free_que, DirtyList& dirty_list
  )
{
  if (owner != this)
  {
    std::size_t id = owner->get_id();
    cache<T, FreeQueue>& cac = cache_list[id];
    cac.push(o, free_que);
    if (cac.size_ >= GCE_FREE_CACHE_SIZE)
    {
      cac.free();
    }
    else if (cac.size_ == 1)
    {
      dirty_list.push_back(&cac);
    }
  }
  else
  {
    pool->free(o);
  }
}
///------------------------------------------------------------------------------
template <typename T, typename Pool, typename FreeQueue>
void cache_pool::free_object(Pool* pool, FreeQueue* free_que)
{
  T* o = free_que->pop_all_reverse();
  while (o)
  {
    T* next = node_access::get_next(o);
    node_access::set_next(o, (T*)0);
    pool->free(o);
    o = next;
  }
}
///------------------------------------------------------------------------------
void cache_pool::free_object()
{
  free_object<actor>(actor_pool_.get(), actor_free_queue_.get());
  free_object<thin>(thin_pool_.get(), thin_free_queue_.get());
  free_object<pack>(pack_pool_.get(), pack_free_queue_.get());
  free_object<socket>(socket_pool_.get(), socket_free_queue_.get());
  free_object<acceptor>(acceptor_pool_.get(), acceptor_free_queue_.get());
}
///------------------------------------------------------------------------------
void cache_pool::free_cache()
{
  if (!actor_cache_dirty_list_.empty())
  {
    BOOST_FOREACH(actor_cache_t* cac, actor_cache_dirty_list_)
    {
      cac->free();
    }
    actor_cache_dirty_list_.clear();
  }

  if (!thin_cache_dirty_list_.empty())
  {
    BOOST_FOREACH(thin_cache_t* cac, thin_cache_dirty_list_)
    {
      cac->free();
    }
    thin_cache_dirty_list_.clear();
  }

  if (!pack_cache_dirty_list_.empty())
  {
    BOOST_FOREACH(pack_cache_t* cac, pack_cache_dirty_list_)
    {
      cac->free();
    }
    pack_cache_dirty_list_.clear();
  }

  if (!socket_cache_dirty_list_.empty())
  {
    BOOST_FOREACH(socket_cache_t* cac, socket_cache_dirty_list_)
    {
      cac->free();
    }
    socket_cache_dirty_list_.clear();
  }

  if (!acceptor_cache_dirty_list_.empty())
  {
    BOOST_FOREACH(acceptor_cache_t* cac, acceptor_cache_dirty_list_)
    {
      cac->free();
    }
    acceptor_cache_dirty_list_.clear();
  }
}
///------------------------------------------------------------------------------
}
}
