///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_ASIO_ALLOC_HANDLER_HPP
#define GCE_DETAIL_ASIO_ALLOC_HANDLER_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/noncopyable.hpp>

#ifndef GCE_ASIO_ALLOC_HANDLER_SIZE
# define GCE_ASIO_ALLOC_HANDLER_SIZE 1024
#endif

namespace gce
{
namespace detail
{
/// handler allocator
template <size_t StorageSize>
class handler_allocator
  : private boost::noncopyable
{
public:
  handler_allocator()
    : in_use_(false)
  {
  }

  ~handler_allocator()
  {
  }

  void* allocate(size_t const size)
  {
    if (!in_use_ && size < storage_.size)
    {
      in_use_ = true;
      return storage_.address();
    }
    else
    {
      return ::operator new(size);
    }
  }

  void deallocate(void* pointer)
  {
    if (pointer == storage_.address())
    {
      in_use_ = false;
    }
    else
    {
      ::operator delete(pointer);
    }
  }

  void on_free()
  {
    in_use_ = false;
  }

private:
  boost::aligned_storage<StorageSize> storage_;
  bool in_use_;
};

/// asio_alloc_handler.
template <typename Handler, size_t StorageSize>
class asio_alloc_handler
{
public:
  typedef handler_allocator<StorageSize> handler_allocator_t;
  typedef asio_alloc_handler<Handler, StorageSize> self_t;

public:
  asio_alloc_handler(handler_allocator_t& alloc, Handler const& handler)
    : allocator_(alloc)
    , handler_(handler)
  {
  }

  ~asio_alloc_handler()
  {
  }

  public:
  void operator()()
  {
    handler_();
  }

  void operator()() const
  {
    handler_();
  }

  template <typename A1>
  void operator()(A1 a1)
  {
    handler_(a1);
  }

  template <typename A1>
  void operator()(A1 a1) const
  {
    handler_(a1);
  }

  template <typename A1, typename A2>
  void operator()(A1 a1, A2 a2)
  {
    handler_(a1, a2);
  }

  template <typename A1, typename A2>
  void operator()(A1 a1, A2 a2) const
  {
    handler_(a1, a2);
  }

  template <typename A1, typename A2, typename A3>
  void operator()(A1 a1, A2 a2, A3 a3)
  {
    handler_(a1, a2, a3);
  }

  template <typename A1, typename A2, typename A3>
  void operator()(A1 a1, A2 a2, A3 a3) const
  {
    handler_(a1, a2, a3);
  }

  template <typename A1, typename A2, typename A3, typename A4>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4)
  {
    handler_(a1, a2, a3, a4);
  }

  template <typename A1, typename A2, typename A3, typename A4>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4) const
  {
    handler_(a1, a2, a3, a4);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    handler_(a1, a2, a3, a4, a5);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
  {
    handler_(a1, a2, a3, a4, a5);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    handler_(a1, a2, a3, a4, a5, a6);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
  {
    handler_(a1, a2, a3, a4, a5, a6);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    handler_(a1, a2, a3, a4, a5, a6, a7);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
  {
    handler_(a1, a2, a3, a4, a5, a6, a7);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7, typename A8>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    handler_(a1, a2, a3, a4, a5, a6, a7, a8);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7, typename A8>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
  {
    handler_(a1, a2, a3, a4, a5, a6, a7, a8);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7, typename A8, typename A9>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    handler_(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7, typename A8, typename A9>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const
  {
    handler_(a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7, typename A8, typename A9, typename A10>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    handler_(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5, typename A6,
        typename A7, typename A8, typename A9, typename A10>
  void operator()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const
  {
    handler_(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

public:
  friend void* asio_handler_allocate(
    size_t const size,
    self_t* this_handler
    )
  {
    return this_handler->allocator_.allocate(size);
  }

  friend void asio_handler_deallocate(
    void* pointer, size_t const /*size*/,
    self_t* this_handler
    )
  {
    this_handler->allocator_.deallocate(pointer);
  }

private:
  handler_allocator_t& allocator_;
  Handler handler_;
};

/// Helper function to wrap a handler object to add custom allocation.
template <typename Handler, size_t StorageSize>
inline asio_alloc_handler<Handler, StorageSize> make_asio_alloc_handler(
  handler_allocator<StorageSize>& a, Handler const& h
  )
{
  return asio_alloc_handler<Handler, StorageSize>(a, h);
}

typedef handler_allocator<GCE_ASIO_ALLOC_HANDLER_SIZE> handler_allocator_t;
}
}

#endif /// GCE_DETAIL_ASIO_ALLOC_HANDLER_HPP
