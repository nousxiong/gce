///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_BUFFER_HPP
#define GCE_DETAIL_BUFFER_HPP

#include <gce/config.hpp>
#include <gce/detail/ref_count.hpp>
#include <boost/bind.hpp>
#include <new>
#include <cstdlib>
#include <cstddef>

namespace gce
{
namespace detail
{
template <typename RefCount>
class basic_buffer
  : public RefCount
{
  typedef basic_buffer<RefCount> self_t;
  struct free_binder
  {
    explicit free_binder(self_t* p)
      : p_(p)
    {
    }

    void operator()() const
    {
      delete p_;
    }

    self_t* p_;
  };

public:
  basic_buffer()
    : RefCount(free_binder(this))
    , data_(0)
    , size_(0)
  {
  }

  explicit basic_buffer(size_t size)
    : RefCount(free_binder(this))
    , data_((byte_t*)std::malloc(size))
    , size_(size)
  {
    if (size > 0 && data_ == 0)
    {
      throw std::bad_alloc();
    }
  }

  basic_buffer(basic_buffer const& other)
    : RefCount(free_binder(this))
    , data_((byte_t*)std::malloc(other.size_))
    , size_(other.size_)
  {
    if (size_ > 0 && data_ == 0)
    {
      throw std::bad_alloc();
    }

    std::memcpy(data_, other.data_, size_);
  }

  basic_buffer& operator=(basic_buffer const& rhs)
  {
    if (this != &rhs)
    {
      if (size_ < rhs.size_)
      {
        data_ = (byte_t*)std::realloc(data_, rhs.size_);
      }
      size_ = rhs.size_;
      std::memcpy(data_, rhs.data_, size_);
    }
    return *this;
  }

  ~basic_buffer()
  {
    if (data_)
    {
      std::free(data_);
    }
  }

public:
  byte_t* data() { return data_; }
  size_t size() const { return size_; }

  void resize(size_t size)
  {
    if (size > size_)
    {
      void* p = std::realloc(data_, size);
      if (!p)
      {
        throw std::bad_alloc();
      }
      data_ = (byte_t*)p;
    }
    size_ = size;
  }

private:
  byte_t* data_;
  size_t size_;
};

typedef basic_buffer<ref_count> buffer;
typedef basic_buffer<ref_count_st> buffer_st;
typedef boost::intrusive_ptr<buffer> buffer_ptr;
}
}

#endif /// GCE_DETAIL_BUFFER_HPP
