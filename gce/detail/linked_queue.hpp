///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_LINKED_QUEUE_HPP
#define GCE_DETAIL_LINKED_QUEUE_HPP

#include <gce/config.hpp>
#include <gce/detail/linked_elem.hpp>

namespace gce
{
namespace detail
{
/// note: T must have default constructor and has a T* next_ member
template <typename T>
class linked_queue
{
public:
  linked_queue()
    : front_(0)
    , back_(0)
    , size_(0)
  {
  }

  ~linked_queue()
  {
  }

public:
  T* pop()
  {
    if (empty())
    {
      return 0;
    }
    else
    {
      T* t = front_;
      front_ = front_->next_;
      if (t == back_)
      {
        back_ = front_;
      }
      t->next_ = 0;
      --size_;
      return t;
    }
  }

  void push(T* t)
  {
    if (empty())
    {
      front_ = t;
      back_ = t;
    }
    else
    {
      back_->next_ = t;
      back_ = t;
    }
    t->next_ = 0;
    ++size_;
  }

  bool empty() const
  {
    return size_ == 0;
  }

  size_t size() const
  {
    return size_;
  }

  T* front()
  {
    return front_;
  }

  T* back()
  {
    return back_;
  }

private:
  T* front_;
  T* back_;
  size_t size_;
};
}
}

#endif /// GCE_DETAIL_LINKED_QUEUE_HPP
