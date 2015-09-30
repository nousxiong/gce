///
/// index_table.hpp
///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_INDEX_TABLE_HPP
#define GCE_DETAIL_INDEX_TABLE_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <deque>
#include <vector>

namespace gce
{
namespace detail
{
template <typename T>
class index_table
{
  struct elem
  {
    explicit elem(T const& t)
      : nil_(false)
      , t_(t)
    {
    }
    
    bool nil_;
    T t_;
  };
  
public:
  explicit index_table(T const& t_nil = T())
    : t_nil_(t_nil)
    , size_(0)
  {
  }
  
  ~index_table()
  {
  }
  
public:
  size_t add(T const& t)
  {
    size_t index = size_nil;
    if (nil_list_.empty())
    {
      index = elem_list_.size();
      elem_list_.push_back(elem(t));
    }
    else
    {
      index = nil_list_.back();
      nil_list_.pop_back();
      elem& e = elem_list_[index];
      e.nil_ = false;
      e.t_ = t;
    }
    ++size_;
    return index;
  }
  
  T* get(size_t index)
  {
    T* t = 0;
    if (index < elem_list_.size())
    {
      elem& e = elem_list_[index];
      if (!e.nil_)
      {
        t = &e.t_;
      }
    }
    return t;
  }
  
  void rmv(size_t index)
  {
    if (index < elem_list_.size())
    {
      elem& e = elem_list_[index];
      if (!e.nil_)
      {
        e.nil_ = true;
        e.t_ = t_nil_;
        nil_list_.push_back(index);
        --size_;
      }
    }
  }
  
  bool empty() const
  {
    return size() == 0;
  }
  
  size_t size() const
  {
    return size_;
  }

private:
  T const t_nil_;
  std::deque<elem> elem_list_;
  std::vector<size_t> nil_list_;
  size_t size_;
};
}
}

#endif /// GCE_DETAIL_INDEX_TABLE_HPP
