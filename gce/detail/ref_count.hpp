///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_REF_COUNT_HPP
#define GCE_DETAIL_REF_COUNT_HPP

#include <gce/config.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/atomic.hpp>
#include <boost/function.hpp>

namespace gce
{
namespace detail
{
class ref_count
{
  ref_count();
public:
  template <typename D>
  explicit ref_count(D d) : deleter_(d), count_(0) {}

  template <typename D, typename A>
  ref_count(D d, A a) : deleter_(d, a), count_(0) {}

  virtual ~ref_count() {}

public:
  long use_count() const
  {
    return count_;
  }

  #if !defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
  inline friend void intrusive_ptr_add_ref(ref_count* p)
  {
    p->count_.fetch_add(1, boost::memory_order_relaxed);
  }

  inline friend void intrusive_ptr_release(ref_count* p)
  {
    if (p->count_.fetch_sub(1, boost::memory_order_release) == 1)
    {
      boost::atomic_thread_fence(boost::memory_order_acquire);
      BOOST_ASSERT(p->deleter_);
      p->deleter_();
    }
  }
#endif

  void add_ref()
  {
    count_.fetch_add(1, boost::memory_order_relaxed);
  }

  void release()
  {
    if (count_.fetch_sub(1, boost::memory_order_release) == 1)
    {
      boost::atomic_thread_fence(boost::memory_order_acquire);
      BOOST_ASSERT(deleter_);
      deleter_();
    }
  }

private:
  boost::function<void ()> deleter_;
  boost::atomic_long count_;
};
}
}

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)

namespace boost
{
inline void intrusive_ptr_add_ref(gce::detail::ref_count* p)
{
  p->add_ref();
}

inline void intrusive_ptr_release(gce::detail::ref_count* p)
{
  p->release();
}
} // namespace boost

#endif

namespace gce
{
namespace detail
{
/// single-thread ref count
class ref_count_st
{
  ref_count_st();
public:
  template <typename D>
  explicit ref_count_st(D d) : deleter_(d), count_(0) {}

  template <typename D, typename A>
  ref_count_st(D d, A a) : deleter_(d, a), count_(0) {}

  virtual ~ref_count_st() {}

public:
  long use_count() const
  {
    return count_;
  }

  #if !defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)
  inline friend void intrusive_ptr_add_ref(ref_count_st* p)
  {
    ++p->count_;
  }

  inline friend void intrusive_ptr_release(ref_count_st* p)
  {
    if (--p->count_ == 0)
    {
      BOOST_ASSERT(p->deleter_);
      p->deleter_();
    }
  }
#endif

  void add_ref()
  {
    ++count_;
  }

  void release()
  {
    if (--count_ == 0)
    {
      BOOST_ASSERT(deleter_);
      deleter_();
    }
  }

private:
  boost::function<void ()> deleter_;
  long count_;
};
}
}

#if defined(BOOST_NO_ARGUMENT_DEPENDENT_LOOKUP)

namespace boost
{
inline void intrusive_ptr_add_ref(gce::detail::ref_count_st* p)
{
  p->add_ref();
}

inline void intrusive_ptr_release(gce::detail::ref_count_st* p)
{
  p->release();
}
} // namespace boost

#endif

#endif /// GCE_DETAIL_REF_COUNT_HPP
