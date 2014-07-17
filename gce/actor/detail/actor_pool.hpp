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
  typedef T value_type;
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
      BOOST_ASSERT(sid_ == sid_nil);
    }

    inline operator bool() const
    {
      return sid_ != sid_nil;
    }

    inline bool operator!() const
    {
      return sid_ == sid_nil;
    }

    inline void use(sid_t sid)
    {
      sid_ = sid;
    }

    inline void unuse()
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
    boost::uint16_t owner,
    std::size_t reserve_size = 32,
    allocator_t a = allocator_t()
    )
    : ctxid_(ctxid)
    , timestamp_(timestamp)
    , owner_(owner)
    , sid_base_(0)
  {
    pool_.resize(reserve_size);
    for (boost::uint32_t i=0; i<reserve_size; ++i)
    {
      free_list_.push_back(i);
    }
  }

  ~actor_pool()
  {
  }

public:
  inline pointer make()
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first);
  }

  template <typename A1>
  inline pointer make(A1 a1)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1);
  }

  template <typename A1, typename A2>
  inline pointer make(A1 a1, A2 a2)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2);
  }

  template <typename A1, typename A2, typename A3>
  inline pointer make(A1 a1, A2 a2, A3 a3)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2, a3);
  }

  template <typename A1, typename A2, typename A3, typename A4>
  inline pointer make(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2, a3, a4);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  inline pointer make(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    std::pair<gce::aid_t, actor_index> pr = get();
    return new ((pointer)pool_[pr.second.id_].segment_) T(pr.first, a1, a2, a3, a4, a5);
  }

  inline pointer get(actor_index i, sid_t sid)
  {
    pointer p = 0;
    if (
      i && i.id_ < pool_.size() && pool_[i.id_] && 
      pool_[i.id_].sid_ == sid
      )
    {
      p = (pointer)pool_[i.id_].segment_;
    }
    return p;
  }

  inline void free(pointer a)
  {
    if (a)
    {
      a->~value_type();
      free(a->get_aid());
    }
  }

private:
  inline std::pair<gce::aid_t, actor_index> get()
  {
    actor_index i;
    if (free_list_.empty())
    {
      i.id_ = (boost::uint32_t)pool_.size();
      pool_.push_back(object());
    }
    else
    {
      boost::uint32_t id = free_list_.back();
      free_list_.pop_back();
      BOOST_ASSERT(id < pool_.size());
      BOOST_ASSERT(!pool_[id]);
      i.id_ = id;
    }

    i.cac_id_ = owner_;
    i.type_ = value_type::type();
    pool_[i.id_].use(++sid_base_);
    return std::make_pair(gce::aid_t(ctxid_, timestamp_, i.id_, i.cac_id_, i.type_, sid_base_), i);
  }

  inline void free(gce::aid_t aid)
  {
    if (aid)
    {
      actor_index i = aid.get_actor_index(ctxid_, timestamp_);
      if (i)
      {
        BOOST_ASSERT(i.id_ < pool_.size());
        BOOST_ASSERT(pool_[i.id_]);
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
  boost::uint16_t owner_;

  std::deque<object> pool_;
  std::vector<boost::uint32_t> free_list_;
  sid_t sid_base_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACTOR_POOL_HPP
