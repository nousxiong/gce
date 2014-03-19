///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_MPSC_QUEUE_HPP
#define GCE_DETAIL_MPSC_QUEUE_HPP

#include <gce/config.hpp>
#include <boost/noncopyable.hpp>
#include <boost/atomic.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/static_assert.hpp>

namespace gce
{
namespace detail
{
class node_access
{
public:
  template <typename T>
  inline static T* get_next(T* n)
  {
    return n->next_node_;
  }

  template <typename T>
  inline static void set_next(T* n, T* next)
  {
    n->next_node_ = next;
  }
};

template <typename T>
class mpsc_queue
  : private boost::noncopyable
{
public:
  typedef T value_type;
  typedef value_type* pointer;

public:
  mpsc_queue() : head_(0) {}

public:
  struct node
  {
    friend class node_access;
    node() : next_node_(0) {}
    virtual ~node() {}

  private:
    pointer next_node_;
  };

public:
  void push(pointer n)
  {
    BOOST_STATIC_ASSERT((boost::is_base_of<node, value_type>::value));

    pointer stale_head = head_.load(boost::memory_order_relaxed);
    do
    {
      node_access::set_next(n, stale_head);
    }
    while (
      !head_.compare_exchange_weak(
        stale_head, n, boost::memory_order_release
        )
      );
  }

  pointer pop_all()
  {
    BOOST_STATIC_ASSERT((boost::is_base_of<node, value_type>::value));

    pointer last = pop_all_reverse();
    pointer first = 0;
    while (last)
    {
      pointer tmp = last;
      last = node_access::get_next(last);
      node_access::set_next(tmp, first);
      first = tmp;
    }
    return first;
  }

  /// Alternative interface if ordering is of no importance.
  pointer pop_all_reverse()
  {
    BOOST_STATIC_ASSERT((boost::is_base_of<node, value_type>::value));
    return head_.exchange(0, boost::memory_order_acquire);
  }

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  GCE_CACHE_ALIGNED_VAR(boost::atomic<pointer>, head_)
};
}
}

#endif /// GCE_DETAIL_MPSC_QUEUE_HPP
