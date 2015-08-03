///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_LINKED_POOL_HPP
#define GCE_DETAIL_LINKED_POOL_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <gce/detail/linked_elem.hpp>
#include <utility>

namespace gce
{
namespace detail
{
/// note: T must have default constructor and has a T* next_ member
template <typename T>
class linked_pool
{
public:
  struct line
  {
    line()
      : front_(0)
      , back_(0)
      , size_(0)
    {
    }

    line(T* front, T* back, size_t size)
      : front_(front)
      , back_(back)
      , size_(size)
    {

    }

    T* front_;
    T* back_;
    size_t size_;
  };

public:
  explicit linked_pool(size_t const reserve_size = 0, size_t const max_size = size_nil)
    : curr_(0)
    , size_(0)
    , max_size_(max_size)
#ifdef GCE_POOL_CHECK
    , check_(0)
#endif
  {
    for (size_t i=0; i<reserve_size; ++i)
    {
      T* t = new T;
      add(t);
    }
  }

  ~linked_pool()
  {
    while (!empty())
    {
      T* t = curr_;
      curr_ = curr_->next_;
      delete t;
    }

#ifdef GCE_POOL_CHECK
    GCE_ASSERT(check_ == 0);
#endif
  }

public:
  T* try_get()
  {
    if (empty())
    {
      return 0;
    }
    else
    {
      T* t = curr_;
      curr_ = curr_->next_;
      t->next_ = 0;
      --size_;
#ifdef GCE_POOL_CHECK
      --check_;
#endif
      return t;
    }
  }

  T* get()
  {
    T* t = try_get();
    if (t == 0)
    {
      t = new T;
#ifdef GCE_POOL_CHECK
      --check_;
#endif
    }
    return t;
  }

  void free(T* t)
  {
#ifdef GCE_POOL_CHECK
    ++check_;
#endif
    if (size_ >= max_size_)
    {
      delete t;
      return;
    }

    t->on_free();
    add(t);
  }

  line detach(size_t count = size_nil)
  {
    T* front = curr_;
    T* back = curr_;
    size_t size = 0;
    for (; size<count && !empty(); ++size)
    {
      back = curr_;
      curr_ = curr_->next_;
    }
    size_ -= size;
    if (back != 0)
    {
      back->next_ = 0;
    }
#ifdef GCE_POOL_CHECK
    check_ -= size;
#endif
    return line(front, back, size);
  }

  void attach(line l)
  {
    if (l.front_ == 0 || l.back_ == 0)
    {
      return;
    }

    l.back_->next_ = curr_;
    curr_ = l.front_;
    size_ += l.size_;
#ifdef GCE_POOL_CHECK
    check_ += l.size_;
#endif
  }

  void drop()
  {
    curr_ = 0;
#ifdef GCE_POOL_CHECK
    check_ -= size_;
#endif
    size_ = 0;
  }

  bool empty() const
  {
    return curr_ == 0;
  }

  size_t size() const
  {
    return size_;
  }

private:
  void add(T* t)
  {
    t->next_ = curr_;
    curr_ = t;
    ++size_;
  }

private:
  T* curr_;
  size_t size_;
  size_t const max_size_;

#ifdef GCE_POOL_CHECK
  int check_;
#endif
};
}
}

#endif /// GCE_DETAIL_LINKED_POOL_HPP
