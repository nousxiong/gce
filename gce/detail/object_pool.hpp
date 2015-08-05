///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_OBJECT_POOL_HPP
#define GCE_DETAIL_OBJECT_POOL_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <gce/detail/linked_pool.hpp>
#include <gce/detail/dynarray.hpp>
#include <boost/container/deque.hpp>
#include <boost/assert.hpp>

namespace gce
{
namespace detail
{
template <typename T>
class object_pool
{
public:
  object_pool(size_t const reserve_size = 0, size_t const grow_size = 32)
    : reserve_size_(reserve_size)
    , grow_size_(grow_size)
  {
    BOOST_ASSERT(grow_size != size_nil);
    grow(reserve_size);
  }

  ~object_pool()
  {
    pool_.drop();
  }

public:
  T* get()
  {
    T* t = pool_.try_get();
    if (t == 0)
    {
      grow(grow_size_);
      t = pool_.try_get();
    }
    return t;
  }

  void free(T* t)
  {
    pool_.free(t);
  }

  size_t size() const
  {
    return pool_.size();
  }

  size_t capacity() const
  {
    if (data_.empty())
    {
      return 0;
    }
    else
    {
      return reserve_size_ + grow_size_ * (data_.size() - 1);
    }
  }

private:
  void grow(size_t size)
  {
    if (size == 0)
    {
      return;
    }
    data_.emplace_back(size);
    dynarray<T>& arr = data_.back();
    for (size_t i=0; i<size; ++i)
    {
      arr.emplace_back();
      pool_.free(&arr.back());
    }
  }

private:
  size_t const reserve_size_;
  size_t const grow_size_;
  boost::container::deque<dynarray<T> > data_;
  linked_pool<T> pool_;
};
}
}

#endif /// GCE_DETAIL_OBJECT_POOL_HPP
