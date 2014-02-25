///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_OBJECT_POOL_HPP
#define GCE_ACTOR_DETAIL_OBJECT_POOL_HPP

#include <gce/actor/config.hpp>
#include <gce/detail/object_access.hpp>
#include <gce/detail/freelist.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <memory>

namespace gce
{
namespace detail
{
class cache_pool;
template <
  typename T,
  typename Args = void,
  typename Alloc = std::allocator<T>
  >
class object_pool
{
  typedef typename detail::freelist<T, Args, Alloc> pool_t;
  typedef typename pool_t::value_type value_type;
  typedef typename pool_t::args_t args_t;
  typedef typename pool_t::allocator_t allocator_t;
  typedef typename pool_t::pointer pointer;

public:
  struct object
  {
    friend class object_access;
    object() : next_obj_(0) {}
    virtual ~object() {}
    virtual void on_free() {}

  private:
    pointer next_obj_;
  };

public:
  explicit object_pool(
    cache_pool* owner,
    args_t args,
    std::size_t cache_size = 32, /// size_nil means always cache object
    std::size_t reserve_size = 32,
    allocator_t a = allocator_t()
    )
    : owner_(owner)
    , pool_(args, cache_size, reserve_size, a)
  {
  }

  explicit object_pool(
    cache_pool* owner,
    std::size_t cache_size = 32, /// size_nil means always cache object
    std::size_t reserve_size = 32,
    allocator_t a = allocator_t()
    )
    : owner_(owner)
    , pool_(cache_size, reserve_size, a)
  {
  }

  ~object_pool()
  {
  }

public:
  inline pointer get()
  {
    BOOST_STATIC_ASSERT((boost::is_base_of<object, value_type>::value));

    return pool_.get();
  }

  inline void free(pointer obj)
  {
    BOOST_STATIC_ASSERT((boost::is_base_of<object, value_type>::value));

    pool_.free(obj);
  }

  inline cache_pool* get_owner() { return owner_; }

private:
  cache_pool* owner_;
  pool_t pool_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_OBJECT_POOL_HPP
