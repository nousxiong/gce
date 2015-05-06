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
class buffer
  : public ref_count
{
public:
  buffer()
    : ref_count(boost::bind(&buffer::free, this))
    , data_(0)
    , size_(0)
  {
  }

  explicit buffer(size_t size)
    : ref_count(boost::bind(&buffer::free, this))
    , data_((byte_t*)std::malloc(size))
    , size_(size)
  {
    if (size > 0 && !data_)
    {
      throw std::bad_alloc();
    }
  }

  ~buffer()
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

  void free()
  {
    delete this;
  }

private:
  byte_t* data_;
  size_t size_;
};

typedef boost::intrusive_ptr<buffer> buffer_ptr;
}
}

#endif /// GCE_DETAIL_BUFFER_HPP
