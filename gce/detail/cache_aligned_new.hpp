#ifndef GCE_DETAIL_CACHE_ALIGNED_NEW_HPP
#define GCE_DETAIL_CACHE_ALIGNED_NEW_HPP

#include <gce/config.hpp>
#include <gce/detail/cache_aligned_allocator.hpp>

namespace gce
{
namespace detail
{
struct cache_aligned_maker
{
  template <typename T>
  static cache_aligned_allocator<T>& get_allocator()
  {
    /// We don't need worry about "multi-instance problem" here
    /// Because cache_aligned_allocator<> is stateless.
    static cache_aligned_allocator<T> ret;
    return ret;
  }
};
}
}

#define GCE_CACHE_ALIGNED_ALLOC(T, n) gce::detail::cache_aligned_maker::get_allocator<T>().allocate(n)
#define GCE_CACHE_ALIGNED_DEALLOC(T, p, n) gce::detail::cache_aligned_maker::get_allocator<T>().deallocate(p, n)

#define GCE_CACHE_ALIGNED_NEW(T) \
  new ((void*)(gce::detail::cache_aligned_maker::get_allocator<T>().allocate(1))) T
#define GCE_CACHE_ALIGNED_DELETE(T, p) \
  if (p) \
  { \
    gce::detail::cache_aligned_allocator<T>& a = gce::detail::cache_aligned_maker::get_allocator<T>(); \
    a.destroy(p); \
    a.deallocate(p, 1); \
  }

namespace gce
{
namespace detail
{
struct cache_aligned_deleter
{
  template <typename T>
  void operator()(T* p)
  {
    GCE_CACHE_ALIGNED_DELETE(T, p);
  }
};
}
}

#endif /// GCE_DETAIL_CACHE_ALIGNED_NEW_HPP
