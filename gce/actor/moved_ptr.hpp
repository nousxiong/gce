///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_MOVE_PTR_HPP
#define GCE_ACTOR_MOVE_PTR_HPP

#include <gce/actor/config.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/core/checked_delete.hpp>
#include <boost/assert.hpp>
#include <utility>

namespace gce
{
template <typename T>
struct default_move_deleter
{
  void operator()(void* o) const 
  {
    T* t = static_cast<T*>(o);
    boost::checked_delete(t);
  }
};

/// move when copy or assign, intrusive ptr
template <typename T>
class moved_ptr
{
  typedef boost::function<void (void*)> deleter_t;

public:
  moved_ptr()
    : p_(0)
  {
  }
  
  template<class Y>
  explicit moved_ptr(Y* p)
    : p_(p)
    , d_(default_move_deleter<T>())
  {

  }
  
  template<class Y, class D>
  explicit moved_ptr(Y* p, D d)
    : p_(p)
    , d_(d)
  {
  }
  
  template<class Y>
  moved_ptr(moved_ptr<Y> const& other)
    : p_(other.p_)
    , d_(other.d_)
  {
    other.release();
  }
  
  moved_ptr(moved_ptr const& other)
    : p_(other.p_)
    , d_(other.d_)
  {
    other.release();
  }
  
  template<class Y>
  moved_ptr(moved_ptr<Y> const& other, T* p)
    : p_(p)
    , d_(other.d_)
  {
    other.release();
  }
  
  template<class Y>
  moved_ptr& operator=(moved_ptr<Y> const& rhs)
  {
    BOOST_ASSERT(p_ != rhs.p_);
    dispose();
    p_ = rhs.p_;
    d_ = rhs.d_;
    rhs.release();
    return *this;
  }
  
  moved_ptr& operator=(moved_ptr const& rhs)
  {
    if (this != &rhs)
    {
      BOOST_ASSERT(p_ != rhs.p_);
      dispose();
      p_ = rhs.p_;
      d_ = rhs.d_;
      rhs.release();
    }
    return *this;
  }
  
  ~moved_ptr()
  {
    dispose();
  }
  
public:
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
  
private:
  void release() const
  {
    p_ = 0;
    d_.clear();
  }
  
  void dispose()
  {
    if (p_)
    {
      d_(p_);
    }
    release();
  }

private:
  template<class Y> friend class moved_ptr;
  mutable T* p_;
  mutable deleter_t d_;
};

template <>
class moved_ptr<void>
{
  typedef boost::function<void (void*)> deleter_t;

public:
  moved_ptr()
    : p_(0)
  {
  }
  
  template<class Y, class D>
  explicit moved_ptr(Y* p, D d)
    : p_(p)
    , d_(d)
  {
  }
  
  template<class Y>
  moved_ptr(moved_ptr<Y> const& other)
    : p_(other.p_)
    , d_(other.d_)
  {
    other.release();
  }
  
  moved_ptr(moved_ptr const& other)
    : p_(other.p_)
    , d_(other.d_)
  {
    other.release();
  }
  
  template<class Y>
  moved_ptr(moved_ptr<Y> const& other, void* p)
    : p_(p)
    , d_(other.d_)
  {
    other.release();
  }
  
  template<class Y>
  moved_ptr& operator=(moved_ptr<Y> const& rhs)
  {
    BOOST_ASSERT(p_ != rhs.p_);
    dispose();
    p_ = rhs.p_;
    d_ = rhs.d_;
    rhs.release();
    return *this;
  }
  
  moved_ptr& operator=(moved_ptr const& rhs)
  {
    if (this != &rhs)
    {
      BOOST_ASSERT(p_ != rhs.p_);
      dispose();
      p_ = rhs.p_;
      d_ = rhs.d_;
      rhs.release();
    }
    return *this;
  }
  
  ~moved_ptr()
  {
    dispose();
  }
  
public:
  void* get() const
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
  
private:
  void release() const
  {
    p_ = 0;
    d_ = deleter_t();
  }
  
  void dispose()
  {
    if (p_)
    {
      d_(p_);
    }
    release();
  }

private:
  template<class Y> friend class moved_ptr;
  mutable void* p_;
  mutable deleter_t d_;
};

template<class T, class U> moved_ptr<T> static_pointer_cast(moved_ptr<U> const& r) BOOST_NOEXCEPT
{
  (void) static_cast<T*>(static_cast<U*>(0));

  T* p = static_cast<T*>(r.get());
  return moved_ptr<T>(r, p);
}
}

#endif // GCE_ACTOR_MOVE_PTR_HPP
