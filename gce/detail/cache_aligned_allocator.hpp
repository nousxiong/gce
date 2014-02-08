///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_CACHE_ALIGNED_ALLOCATOR_HPP
#define GCE_DETAIL_CACHE_ALIGNED_ALLOCATOR_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <boost/assert.hpp>
#include <memory>
#include <cstdlib>

namespace gce
{
namespace detail
{
struct default_malloc_free
{
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  void* malloc(size_type const size)
  {
    return std::malloc(size);
  }

  void free(void* ptr)
  {
    if (ptr)
    {
      std::free(ptr);
    }
  }
};

template <typename T, typename M = default_malloc_free>
class cache_aligned_allocator
{
  template<typename U, typename R>
  friend class cache_aligned_allocator;
  template<typename V, typename U, typename R>
  friend bool operator==(cache_aligned_allocator<V,R> const&, cache_aligned_allocator<U,R> const&);
  template<typename V, typename U, typename R>
  friend bool operator!=(cache_aligned_allocator<V,R> const&, cache_aligned_allocator<U,R> const&);

public:
  typedef T value_type;
  typedef value_type* pointer;
  typedef value_type const* const_pointer;
  typedef value_type& reference;
  typedef value_type const& const_reference;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;
  typedef M malloc_t;
  template <typename U>
  struct rebind
  {
    typedef cache_aligned_allocator<U, M> other;
  };

  cache_aligned_allocator() throw() {}
  cache_aligned_allocator(malloc_t const& m) throw() : m_(m) {} /// M's copy constructor cannot throw.
  cache_aligned_allocator(cache_aligned_allocator const& other) throw() : m_(other.m_) {} /// malloc_t's copy constructor cannot throw.
  template<typename U>
  cache_aligned_allocator(cache_aligned_allocator<U, M> const& other) throw() : m_(other.m_) {}

  pointer address(reference x) const {return &x;}
  const_pointer address(const_reference x) const {return &x;}

  /// Allocate space for n objects, starting on a cache/sector line.
  pointer allocate(size_type n, const_pointer hint = 0)
  {
    int line_size = GCE_CACHE_LINE_SIZE;
    size_t bytes = n * sizeof(value_type);

    byte_t* ret = 0;
    byte_t* ptr = (byte_t*)m_.malloc(bytes + line_size);
    /// Since tbb(https://www.threadingbuildingblocks.org/)
    ret = (byte_t*)((uintptr_t)(ptr + line_size) & -line_size);
    ((uintptr_t*)ret)[-1] = uintptr_t(ptr);
    return (pointer)ret;
  }

  /// Free block of memory that starts on a cache line
  void deallocate(pointer p, size_type n)
  {
    if (p)
    {
      BOOST_ASSERT_MSG((uintptr_t)p>=0x4096, "attempt to free block not obtained from cache_aligned_allocator" );
      /// Recover where block actually starts
      byte_t* ptr = ((byte_t**)p)[-1];
      int line_size = GCE_CACHE_LINE_SIZE;
      BOOST_ASSERT_MSG((void*)((uintptr_t)(ptr + line_size) & -line_size) == p, "not allocated by cache_aligned_allocator" );
      m_.free(ptr);
    }
  }

  /// Largest value for which method allocate might succeed.
  size_type max_size() const throw()
  {
    int line_size = GCE_CACHE_LINE_SIZE;
    return (~size_t(0) - line_size) / sizeof(value_type);
  }

  /// Copy-construct value at location pointed to by p.
  void construct(pointer p, const_reference value)
  {
    ::new ((void*)(p)) value_type(value);
  }

  /// Destroy value at location pointed to by p.
  void destroy(pointer p)
  {
    p->~value_type();
  }

private:
  malloc_t m_;
};

template<typename M>
class cache_aligned_allocator<void, M>
{
public:
  typedef void* pointer;
  typedef void const* const_pointer;
  typedef void value_type;
  template <typename U>
  struct rebind
  {
    typedef cache_aligned_allocator<U, M> other;
  };
};

template<typename T, typename U, typename M>
inline bool operator==(cache_aligned_allocator<T, M> const&, cache_aligned_allocator<U, M> const&)
{
  return true;
}

template<typename T, typename U, typename M>
inline bool operator!=(cache_aligned_allocator<T, M> const&, cache_aligned_allocator<U, M> const&)
{
  return false;
}
}
}

#endif /// GCE_DETAIL_CACHE_ALIGNED_ALLOCATOR_HPP
