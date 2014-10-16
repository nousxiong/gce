///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SCOPED_BOOL_HPP
#define GCE_ACTOR_DETAIL_SCOPED_BOOL_HPP

#include <gce/actor/config.hpp>

namespace gce
{
namespace detail
{
template <typename Bool>
struct scoped_bool
{
  explicit scoped_bool(Bool& flag, Bool val = true)
    : flag_(flag)
  {
    flag_ = val;
  }

  ~scoped_bool()
  {
    flag_ = !flag_;
  }

  Bool& flag_;
};

template <>
struct scoped_bool<boost::atomic_bool>
{
  explicit scoped_bool(
    boost::atomic_bool& flag,
    bool val = true,
    boost::memory_order begin_order = boost::memory_order_release,
    boost::memory_order end_order = boost::memory_order_release
    )
    : flag_(flag)
    , val_(!val)
    , end_order_(end_order)
  {
    flag_.store(val, begin_order);
  }

  ~scoped_bool()
  {
    flag_.store(val_, end_order_);
  }

  boost::atomic_bool& flag_;
  bool val_;
  boost::memory_order end_order_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_SCOPED_BOOL_HPP
