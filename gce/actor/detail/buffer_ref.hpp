///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_BUFFER_REF_HPP
#define GCE_ACTOR_DETAIL_BUFFER_REF_HPP

#include <gce/actor/config.hpp>

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

  buffer_ref(byte_t* buf, std::size_t size)
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
  std::size_t remain_write_size() const { return size_ - write_size_; }
  std::size_t remain_read_size() const { return write_size_ - read_size_; }
  std::size_t write_size() const { return write_size_; }
  std::size_t read_size() const { return read_size_; }
  byte_t* get_write_data() { return buf_ + write_size_; }
  byte_t* get_read_data() { return buf_ + read_size_; }
  byte_t const* data() const { return buf_; }
  std::size_t size() const { return size_; }

  void clear()
  {
    write_size_ = 0;
    read_size_ = 0;
  }

  void clear_read()
  {
    read_size_ = 0;
  }

  void clear_write()
  {
    write_size_ = 0;
  }

  void reset(byte_t* buf, std::size_t size)
  {
    buf_ = buf;
    size_ = size;

    if (read_size_ > size_)
    {
      throw std::runtime_error("read buffer overflow");
    }

    if (write_size_ > size_)
    {
      throw std::runtime_error("write buffer overflow");
    }
  }

  void read(std::size_t size)
  {
    if (read_size_ + size > write_size_)
    {
      throw std::runtime_error("read buffer overflow");
    }

    read_size_ += size;
  }

  void write(std::size_t size)
  {
    if (write_size_ + size > size_)
    {
      throw std::runtime_error("write buffer overflow");
    }

    write_size_ += size;
  }

private:
  byte_t* buf_;
  std::size_t size_;
  std::size_t write_size_;
  std::size_t read_size_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_BUFFER_REF_HPP
