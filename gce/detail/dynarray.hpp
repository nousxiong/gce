///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_DYNARRAY_HPP
#define GCE_DETAIL_DYNARRAY_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <vector>

namespace gce
{
namespace detail
{
template <
  typename T,
  typename Alloc = std::allocator<T>
  >
class dynarray
{
  typedef Alloc allocator_t;
  typedef T* pointer;
  typedef T& reference;
  typedef T const& const_reference;

  struct element
  {
    element()
      : p_(0)
    {
    }

    byte_t segment_[sizeof(T)];
    pointer p_;
  };

public:
  explicit dynarray(size_t max_size, allocator_t a = allocator_t())
    : elem_list_(max_size, element(), a)
    , size_(0)
  {
  }

  ~dynarray()
  {
    clear();
  }

public:
  size_t size() const
  {
    return size_;
  }

  size_t capacity() const
  {
    return elem_list_.size();
  }

  bool empty() const
  {
    return size() == 0;
  }

  reference front()
  {
    return *elem_list_.front().p_;
  }

  const_reference front() const
  {
    return *elem_list_.front().p_;
  }

  reference back()
  {
    return *elem_list_[size_ - 1].p_;
  }

  const_reference back() const
  {
    return *elem_list_[size_ - 1].p_;
  }

  void emplace_back()
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    element& e = elem_list_[size_];
    pointer p = new ((pointer)e.segment_) T;
    e.p_ = p;
    ++size_;
  }
  
  template <typename A1>
  void emplace_back(A1 a1)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    element& e = elem_list_[size_];
    pointer p = new ((pointer)e.segment_) T(a1);
    e.p_ = p;
    ++size_;
  }
  
  template <typename A1, typename A2>
  void emplace_back(A1 a1, A2 a2)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    element& e = elem_list_[size_];
    pointer p = new ((pointer)e.segment_) T(a1, a2);
    e.p_ = p;
    ++size_;
  }

  template <typename A1, typename A2, typename A3>
  void emplace_back(A1 a1, A2 a2, A3 a3)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    element& e = elem_list_[size_];
    pointer p = new ((pointer)e.segment_) T(a1, a2, a3);
    e.p_ = p;
    ++size_;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  void emplace_back(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    element& e = elem_list_[size_];
    pointer p = new ((pointer)e.segment_) T(a1, a2, a3, a4);
    e.p_ = p;
    ++size_;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void emplace_back(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    element& e = elem_list_[size_];
    pointer p = new ((pointer)e.segment_) T(a1, a2, a3, a4, a5);
    e.p_ = p;
    ++size_;
  }

  reference operator[](size_t i)
  {
    return *elem_list_[i].p_;
  }

  const_reference operator[](size_t i) const
  {
    return *elem_list_[i].p_;
  }

  reference at(size_t i)
  {
    return *elem_list_.at(i).p_;
  }

  const_reference at(size_t i) const
  {
    return *elem_list_.at(i).p_;
  }

  void clear()
  {
    for (size_t i=size_; i>0; --i)
    {
      element& e = elem_list_[i-1];
      pointer p = e.p_;
      e.p_ = 0;
      p->~T();
    }
    size_ = 0;
  }

private:
  std::vector<element, allocator_t> elem_list_;
  size_t size_;
};
}
}

#endif /// GCE_DETAIL_DYNARRAY_HPP
