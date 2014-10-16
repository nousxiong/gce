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

  struct memory
  {
    byte_t segment_[sizeof(T)];
  };

public:
  explicit dynarray(std::size_t max_size, allocator_t a = allocator_t())
    : elem_list_(max_size, memory(), a)
    , size_(0)
  {
  }

  ~dynarray()
  {
    clear();
  }

public:
  std::size_t size() const
  {
    return size_;
  }

  std::size_t capacity() const
  {
    return elem_list_.size();
  }

  bool empty() const
  {
    return size() == 0;
  }

  T& make_back()
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    pointer p = new ((pointer)elem_list_[size_].segment_) T;
    ++size_;
    return *p;
  }
  
  template <typename A1>
  T& make_back(A1 a1)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    pointer p = new ((pointer)elem_list_[size_].segment_) T(a1);
    ++size_;
    return *p;
  }
  
  template <typename A1, typename A2>
  T& make_back(A1 a1, A2 a2)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    pointer p = new ((pointer)elem_list_[size_].segment_) T(a1, a2);
    ++size_;
    return *p;
  }

  template <typename A1, typename A2, typename A3>
  T& make_back(A1 a1, A2 a2, A3 a3)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    pointer p = new ((pointer)elem_list_[size_].segment_) T(a1, a2, a3);
    ++size_;
    return *p;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  T& make_back(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    pointer p = new ((pointer)elem_list_[size_].segment_) T(a1, a2, a3, a4);
    ++size_;
    return *p;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  T& make_back(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    if (size_ == elem_list_.size())
    {
      throw std::out_of_range("out of max size");
    }

    pointer p = new ((pointer)elem_list_[size_].segment_) T(a1, a2, a3, a4, a5);
    ++size_;
    return *p;
  }

  T& operator[](std::size_t i)
  {
    return (T&)(*elem_list_[i].segment_);
  }

  T const& operator[](std::size_t i) const
  {
    return (T const&)(*elem_list_[i].segment_);
  }

  T& at(std::size_t i)
  {
    return (T&)(*elem_list_.at(i).segment_);
  }

  T const& at(std::size_t i) const
  {
    return (T const&)(*elem_list_.at(i).segment_);
  }

  void clear()
  {
    for (std::size_t i=size_; i>0; --i)
    {
      pointer p = (pointer)elem_list_[i-1].segment_;
      p->~T();
    }
    size_ = 0;
  }

private:
  std::vector<memory, allocator_t> elem_list_;
  std::size_t size_;
};
}
}

#endif /// GCE_DETAIL_DYNARRAY_HPP
