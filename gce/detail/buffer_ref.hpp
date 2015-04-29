///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_BUFFER_REF_HPP
#define GCE_DETAIL_BUFFER_REF_HPP

#include <gce/config.hpp>
#include <cstddef>
#include <cassert>

namespace gce
{
namespace detail
{
class buffer_ref
{
  buffer_ref(buffer_ref const&);
  buffer_ref& operator=(buffer_ref const& rhs);

public:
  buffer_ref()
    : buf_(0)
    , size_(0)
    , write_size_(0)
    , read_size_(0)
  {
  }

  buffer_ref(byte_t* buf, size_t size)
    : buf_(buf)
    , size_(size)
    , write_size_(0)
    , read_size_(0)
  {
  }

  ~buffer_ref()
  {
  }

public:
  size_t remain_write_size() const { return size_ - write_size_; }
  size_t remain_read_size() const { return write_size_ - read_size_; }
  size_t write_size() const { return write_size_; }
  size_t read_size() const { return read_size_; }
  byte_t* get_write_data() { return buf_ + write_size_; }
  byte_t* get_read_data() { return buf_ + read_size_; }
  byte_t const* data() const { return buf_; }
  size_t size() const { return size_; }

  void clear()
  {
    write_size_ = 0;
    read_size_ = 0;
  }

  byte_t const* clear_read(size_t len = size_nil)
  {
    if (len <= read_size_)
    {
      read_size_ -= len;
    }
    else
    {
      read_size_ = 0;
    }
    return buf_ + read_size_;
  }

  byte_t* clear_write(size_t len = size_nil)
  {
    if (len <= write_size_)
    {
      write_size_ -= len;
    }
    else
    {
      write_size_ = 0;
    }
    assert(read_size_ <= write_size_);
    return buf_ + write_size_;
  }

  void reset(byte_t* buf, size_t size)
  {
    if (read_size_ > size_)
    {
      throw std::out_of_range("read buffer overflow");
    }

    if (write_size_ > size_)
    {
      throw std::out_of_range("write buffer overflow");
    }

    buf_ = buf;
    size_ = size;
  }

  void read(size_t size)
  {
    if (read_size_ + size > write_size_)
    {
      throw std::out_of_range("read buffer overflow");
    }

    read_size_ += size;
  }

  void write(size_t size)
  {
    if (write_size_ + size > size_)
    {
      throw std::out_of_range("write buffer overflow");
    }

    write_size_ += size;
  }

private:
  byte_t* buf_;
  size_t size_;
  size_t write_size_;
  size_t read_size_;
};
}
}

#endif /// GCE_DETAIL_BUFFER_REF_HPP
