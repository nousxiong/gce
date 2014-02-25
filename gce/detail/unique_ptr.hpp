///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_UNIQUE_PTR_HPP
#define GCE_DETAIL_UNIQUE_PTR_HPP

#include <gce/config.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/checked_delete.hpp>
#include <boost/assert.hpp>
#include <utility>

namespace gce
{
namespace detail
{
/// Unique pointer with deleter(like C++11 std::unique_ptr).
/// All member functions never throws.
template <typename T>
class unique_ptr
{
  typedef boost::function<void (T*)> deleter_t;
  unique_ptr(unique_ptr const&);
  unique_ptr& operator=(unique_ptr const&);

  typedef unique_ptr<T> self_t;

  void operator==(unique_ptr const&) const;
  void operator!=(unique_ptr const&) const;

public:
  unique_ptr()
    : px_(0)
  {
  }

  explicit unique_ptr(T* p)
    : px_(p)
  {
  }

  template <class D>
  unique_ptr(T* p, D d)
    : px_(p)
    , deleter_(d)
  {
  }

  ~unique_ptr()
  {
    if (deleter_)
    {
      deleter_(px_);
    }
    else
    {
      boost::checked_delete(px_);
    }
  }

public:
  void reset()
  {
    self_t().swap(*this);
  }

  void reset(T* p)
  {
    BOOST_ASSERT(p == 0 || p != px_);
    self_t(p).swap(*this);
  }

  template <class D>
  void reset(T* p, D d)
  {
    BOOST_ASSERT(p == 0 || p != px_);
    self_t(p, d).swap(*this);
  }

  T& operator*() const
  {
    BOOST_ASSERT(px_ != 0);
    return *px_;
  }

  T* operator->() const
  {
    BOOST_ASSERT(px_ != 0);
    return px_;
  }

  T* get() const
  {
    return px_;
  }

  operator bool() const
  {
    return px_ != 0;
  }

  bool operator!() const
  {
    return px_ == 0;
  }

  void swap(unique_ptr& b)
  {
    std::swap(px_, b.px_);
    std::swap(deleter_, b.deleter_);
  }

private:
  T* px_;
  deleter_t deleter_;
};

template <typename T>
struct empty_deleter
{
  void operator()(T*) const {}
};
}
}

#endif /// GCE_DETAIL_UNIQUE_PTR_HPP
