///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_ACTOR_POOL_HPP
#define GCE_ACTOR_DETAIL_ACTOR_POOL_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>
#include <deque>
#include <vector>
#include <memory>

namespace gce
{
namespace detail
{
class cache_pool;
template <
  typename T,
  typename Alloc = std::allocator<T>
  >
class actor_pool
{
  typedef Alloc allocator_t;
  typedef T* pointer;

  struct object
  {
    object()
      : sid_(sid_nil)
    {
    }

    ~object()
    {
      GCE_ASSERT(sid_ == sid_nil)(sid_);
    }

    operator bool() const
    {
      return sid_ != sid_nil;
    }

    bool operator!() const
    {
      return sid_ == sid_nil;
    }

    void use(sid_t sid)
    {
      sid_ = sid;
    }

    void unuse()
    {
      sid_ = sid_nil;
    }

    sid_t sid_;
    byte_t segment_[sizeof(T)];
  };

public:
  explicit actor_pool(
    ctxid_t ctxid, 
    timestamp_t timestamp, 
    uint16_t owner,
    size_t reserve_size = 32,
    allocator_t a = allocator_t()
    )
    : ctxid_(ctxid)
    , timestamp_(timestamp)
    , owner_(owner)
    , sid_base_(0)
  {
    pool_.resize(reserve_size);
    for (uint32_t i=0; i<reserve_size; ++i)
    {
      free_list_.push_back(i);
    }
  }

  ~actor_pool()
  {
  }

public:
  pointer make()
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first);
  }

  template <typename A1>
  pointer make(A1 a1)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1);
  }

  template <typename A1, typename A2>
  pointer make(A1 a1, A2 a2)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2);
  }

  template <typename A1, typename A2, typename A3>
  pointer make(A1 a1, A2 a2, A3 a3)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2, a3);
  }

  template <typename A1, typename A2, typename A3, typename A4>
  pointer make(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2, a3, a4);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  pointer make(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2, a3, a4, a5);
  }

  pointer find(actor_index i, sid_t sid)
  {
    pointer p = 0;
    if (i && i.id_ < pool_.size())
    {
      object& o = pool_[i.id_];
      if (o && o.sid_ == sid)
      {
        p = (pointer)o.segment_;
      }
    }
    return p;
  }

  void free(pointer a)
  {
    if (a)
    {
      gce::aid_t aid = a->get_aid();
      a->~T();
      free(aid);
    }
  }

private:
  std::pair<gce::aid_t, actor_index> get()
  {
    actor_index i;
    if (free_list_.empty())
    {
      i.id_ = (uint32_t)pool_.size();
      pool_.push_back(object());
    }
    else
    {
      uint32_t id = free_list_.back();
      free_list_.pop_back();
      GCE_ASSERT(id < pool_.size())(id)(pool_.size());
      GCE_ASSERT(!pool_[id])(id);
      i.id_ = id;
    }

    i.svc_id_ = owner_;
    i.type_ = T::get_type();
    pool_[i.id_].use(++sid_base_);
    return std::make_pair(gce::make_aid(ctxid_, timestamp_, i.id_, i.svc_id_, i.type_, sid_base_), i);
  }

  void free(gce::aid_t const& aid)
  {
    if (aid != gce::aid_nil)
    {
      actor_index i = get_actor_index(aid, ctxid_, timestamp_);
      if (i)
      {
        GCE_ASSERT(i.id_ < pool_.size())(i.id_)(pool_.size());
        GCE_ASSERT(pool_[i.id_])(i.id_);
        if (i.id_ + 1 == pool_.size())
        {
          pool_.back().unuse();
          pool_.pop_back();
        }
        else
        {
          pool_[i.id_].unuse();
          free_list_.push_back(i.id_);
        }
      }
    }
  }

private:
  ctxid_t ctxid_;
  timestamp_t timestamp_;
  uint16_t owner_;

  std::deque<object> pool_;
  std::vector<uint32_t> free_list_;
  sid_t sid_base_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACTOR_POOL_HPP
