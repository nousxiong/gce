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
#include <gce/detail/linked_pool.hpp>
#include <deque>
#include <vector>
#include <memory>

namespace gce
{
namespace detail
{
template <typename T>
class actor_pool
{
  typedef T* pointer;

  struct object
    : public linked_elem<object>
  {
    object()
      : sid_(sid_nil)
    {
    }

    object(object const&)
      : sid_(sid_nil)
    {
    }

    object& operator=(object const&)
    {
      return *this;
    }

    ~object()
    {
      GCE_ASSERT(sid_ == sid_nil)(sid_);
    }

    void on_free()
    {
      sid_ = sid_nil;
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

    sid_t sid_;
    byte_t segment_[sizeof(T)];
  };

public:
  explicit actor_pool(
    ctxid_t ctxid, 
    timestamp_t timestamp, 
    uint16_t owner,
    size_t reserve_size = 32,
    size_t max_size = size_nil
    )
    : ctxid_(ctxid)
    , timestamp_(timestamp)
    , owner_(owner)
    , pool_(reserve_size, max_size)
    , sid_base_(0)
  {
  }

  ~actor_pool()
  {
  }

public:
  pointer make()
  {
    aid_t aid = get();
    return new (((object*)aid.uintptr_)->segment_) T(aid);
  }

  template <typename A1>
  pointer make(A1 a1)
  {
    aid_t aid = get();
    return new (((object*)aid.uintptr_)->segment_) T(aid, a1);
  }

  template <typename A1, typename A2>
  pointer make(A1 a1, A2 a2)
  {
    aid_t aid = get();
    return new (((object*)aid.uintptr_)->segment_) T(aid, a1, a2);
  }

  template <typename A1, typename A2, typename A3>
  pointer make(A1 a1, A2 a2, A3 a3)
  {
    aid_t aid = get();
    return new (((object*)aid.uintptr_)->segment_) T(aid, a1, a2, a3);
  }

  template <typename A1, typename A2, typename A3, typename A4>
  pointer make(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    aid_t aid = get();
    return new (((object*)aid.uintptr_)->segment_) T(aid, a1, a2, a3, a4);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  pointer make(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    aid_t aid = get();
    return new (((object*)aid.uintptr_)->segment_) T(aid, a1, a2, a3, a4, a5);
  }

  pointer find(actor_index const& ai, sid_t sid)
  {
    pointer p = 0;
    if (ai)
    {
      object* o = (object*)ai.ptr_;
      if (o->sid_ == sid)
      {
        p = (pointer)o->segment_;
      }
    }
    return p;
  }

  void free(pointer a)
  {
    if (a)
    {
      aid_t aid = a->get_aid();
      a->~T();
      free(aid);
    }
  }

private:
  aid_t get()
  {
    object* o = pool_.get();
    o->use(++sid_base_);
    return make_aid(ctxid_, timestamp_, (uint64_t)o, owner_, T::get_type(), sid_base_);
  }

  void free(aid_t const& aid)
  {
    if (aid != aid_nil)
    {
      GCE_ASSERT(aid.ctxid_ == ctxid_)(aid.ctxid_)(ctxid_);
      GCE_ASSERT(aid.timestamp_ == timestamp_)(aid.timestamp_)(timestamp_);
      pool_.free((object*)aid.uintptr_);
    }
  }

private:
  ctxid_t ctxid_;
  timestamp_t timestamp_;
  uint16_t owner_;
  linked_pool<object> pool_;
  sid_t sid_base_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_ACTOR_POOL_HPP
