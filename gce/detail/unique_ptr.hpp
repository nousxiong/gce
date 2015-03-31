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
#include <boost/core/checked_delete.hpp>
#include <boost/assert.hpp>
#include <utility>

namespace gce
{
namespace detail
{
template <typename T>
struct default_unique_deleter
{
  void operator()(T* t) const 
  {
    boost::checked_delete(t);
  }
};

/// Unique pointer with deleter.
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
    : p_(0)
    , d_(default_unique_deleter<T>())
  {
  }

  explicit unique_ptr(T* p)
    : p_(p)
    , d_(default_unique_deleter<T>())
  {
  }

  template <class D>
  unique_ptr(T* p, D d)
    : p_(p)
    , d_(d)
  {
  }

  ~unique_ptr()
  {
    d_(p_);
  }

public:
  void reset()
  {
    self_t().swap(*this);
  }

  void reset(T* p)
  {
    BOOST_ASSERT(p == 0 || p != p_);
    self_t(p).swap(*this);
  }

  template <class D>
  void reset(T* p, D d)
  {
    BOOST_ASSERT(p == 0 || p != p_);
    self_t(p, d).swap(*this);
  }

  T& operator*() const
  {
    BOOST_ASSERT(p_ != 0);
    return *p_;
  }

  T* operator->() const
  {
    BOOST_ASSERT(p_ != 0);
    return p_;
  }

  T* get() const
  {
    return p_;
  }

  operator bool() const
  {
    return p_ != 0;
  }

  bool operator!() const
  {
    return p_ == 0;
  }

  void swap(unique_ptr& b)
  {
    std::swap(p_, b.p_);
    std::swap(d_, b.d_);
  }

private:
  T* p_;
  deleter_t d_;
};
}
}

#endif /// GCE_DETAIL_UNIQUE_PTR_HPP
