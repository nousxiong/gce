#ifndef GCE_DETAIL_FREELIST_HPP
#define GCE_DETAIL_FREELIST_HPP

#include <gce/config.hpp>
#include <gce/detail/object_access.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_same.hpp>

namespace gce
{
namespace detail
{
template <typename T, typename Args, typename Alloc>
class freelist
{
public:
  typedef T value_type;
  typedef value_type* pointer;

  struct null_args {};
  struct args_tag {};

  typedef typename boost::mpl::if_<
      boost::is_same<Args, void>,
      null_args, Args
    >::type args_t;

  typedef Alloc allocator_t;

private:
  typedef typename boost::mpl::if_<
      boost::is_same<args_t, null_args>,
      null_args, args_tag
    >::type args_select_t;

public:
  freelist(
    args_t args,
    std::size_t cache_size,
    std::size_t reserve_size,
    allocator_t a
    )
    : cache_size_(cache_size)
    , size_(0)
    , args_(args)
    , head_(0)
    , a_(a)
  {
    reserve(reserve_size);
  }

  freelist(
    std::size_t cache_size,
    std::size_t reserve_size,
    allocator_t a
    )
    : cache_size_(cache_size)
    , size_(0)
    , args_(null_args())
    , head_(0)
    , a_(a)
  {
    reserve(reserve_size);
  }

  ~freelist()
  {
    clear_pool();
  }

public:
  pointer get()
  {
    pointer obj = 0;
    if (head_)
    {
      obj = head_;
      pointer next = object_access::get_next(head_);
      head_ = next;
      object_access::set_next(obj, pointer(0));
      --size_;
    }
    else
    {
      obj = make_object(args_select_);
    }
    return obj;
  }

  void free(pointer obj)
  {
    pointer curr = obj;
    while (curr)
    {
      pointer next = object_access::get_next(curr);
      curr->on_free();
      if (size_ < cache_size_)
      {
        object_access::set_next(curr, head_);
        head_ = curr;
        ++size_;
      }
      else
      {
        curr->~value_type();
        a_.deallocate(curr, 1);
      }

      curr = next;
    }
  }

private:
  inline pointer make_object(null_args)
  {
    return new (a_.allocate(1)) value_type;
  }

  inline pointer make_object(args_tag)
  {
    return new (a_.allocate(1)) value_type(args_);
  }

  void reserve(std::size_t size)
  {
    pointer curr = head_;
    try
    {
      for (std::size_t i=0; i<size; ++i)
      {
        pointer obj = make_object(args_select_);
        if (curr)
        {
          object_access::set_next(curr, obj);
        }
        else
        {
          head_ = obj;
        }
        curr = obj;
      }
      size_ += size;
    }
    catch (...)
    {
      clear_pool();
    }
  }

  void clear_pool()
  {
    pointer curr = head_;
    while (curr)
    {
      pointer next = object_access::get_next(curr);
      curr->~value_type();
      a_.deallocate(curr, 1);
      curr = next;
    }
    head_ = 0;
    size_ = 0;
  }

private:
  std::size_t const cache_size_;
  std::size_t size_;

  args_t args_;
  args_select_t args_select_;
  pointer head_;
  allocator_t a_;
};
} /// namespace detail
} /// namespace gce

#endif /// GCE_DETAIL_FREELIST_HPP
