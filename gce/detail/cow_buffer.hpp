///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_COW_BUFFER_HPP
#define GCE_DETAIL_COW_BUFFER_HPP

#include <gce/config.hpp>
#include <gce/detail/buffer.hpp>
#include <gce/detail/buffer_ref.hpp>
#include <boost/array.hpp>
#include <boost/utility/string_ref.hpp>

namespace gce
{
namespace detail
{
template <size_t SmallSize, size_t MinGrowSize = 64>
class cow_buffer
{
public:
  cow_buffer()
    : buf_(small_.data(), small_.size())
  {
  }

  cow_buffer(byte_t const* data, size_t size)
  {
    if (size <= small_.size())
    {
      buf_.reset(small_.data(), small_.size());
    }
    else
    {
      make_large(size);
      buf_.reset(large_->data(), size);
    }
    std::memcpy(buf_.get_write_data(), data, size);
    buf_.write(size);
  }

  cow_buffer(cow_buffer const& other)
  {
    buffer_ref const& buf = other.buf_;
    large_ = other.large_;
    if (other.is_small())
    {
      buf_.reset(small_.data(), small_.size());
      std::memcpy(small_.data(), other.small_.data(), buf.write_size());
    }
    else
    {
      BOOST_ASSERT(large_);
      buf_.reset(large_->data(), large_->size());
    }
    buf_.write(buf.write_size());
  }

  cow_buffer& operator=(cow_buffer const& rhs)
  {
    if (this != &rhs)
    {
      buffer_ref const& buf = rhs.buf_;
      buf_.clear();

      if (rhs.is_small())
      {
        if (!is_small())
        {
          large_.reset();
        }

        buf_.reset(small_.data(), small_.size());
        std::memcpy(small_.data(), rhs.small_.data(), buf.write_size());
      }
      else
      {
        BOOST_ASSERT(rhs.large_);
        large_ = rhs.large_;
        buf_.reset(large_->data(), large_->size());
      }
      buf_.write(buf.write_size());
    }
    return *this;
  }

  ~cow_buffer()
  {
  }

public:
  byte_t const* data() const
  { 
    return buf_.data();
  }

  size_t size() const
  {
    return buf_.write_size();
  }

  buffer_ref& get_buffer_ref()
  { 
    return buf_;
  }

  buffer_ref const& get_buffer_ref() const
  { 
    return buf_;
  }

  bool is_small() const
  {
    return buf_.data() == small_.data();
  }

  void append(std::string const& str)
  {
    append(str.data(), str.size());
  }

  void append(boost::string_ref str)
  {
    append(str.data(), str.size());
  }

  void append(char const* data, size_t size = size_nil)
  {
    if (size == size_nil)
    {
      BOOST_ASSERT(data != 0);
      size = std::char_traits<char>::length(data);
    }
    reserve(size);
    byte_t* write_data = buf_.get_write_data();
    buf_.write(size);
    std::memcpy(write_data, data, size);
  }

  void reserve(size_t size)
  {
    size_t old_buf_capacity = buf_.size();
    size_t old_buf_size = buf_.write_size();
    size_t new_buf_size = old_buf_size + size;
    size_t new_buf_capacity = old_buf_capacity;
    if (new_buf_size > old_buf_capacity)
    {
      size_t diff = new_buf_size - old_buf_capacity;
      if (diff < MinGrowSize)
      {
        diff = MinGrowSize;
      }
      new_buf_capacity = old_buf_capacity + diff;
    }

    if (is_small())
    {
      if (new_buf_capacity > old_buf_capacity)
      {
        make_large(new_buf_capacity);
        std::memcpy(large_->data(), buf_.data(), buf_.write_size());
        buf_.reset(large_->data(), new_buf_capacity);
      }
    }
    else
    {
      BOOST_ASSERT(large_);
      /// copy-on-write
      if (large_->use_count() > 1)
      {
        detail::buffer_ptr tmp = large_;
        make_large(new_buf_capacity);
        std::memcpy(large_->data(), tmp->data(), buf_.write_size());
      }
      else
      {
        large_->resize(new_buf_capacity);
      }
      buf_.reset(large_->data(), new_buf_capacity);
    }
  }

private:
  void make_large(size_t size)
  {
    large_.reset(new buffer(size));
  }

private:
  boost::array<byte_t, SmallSize> small_;
  buffer_ptr large_;
  buffer_ref buf_;
};
}
}

#endif /// GCE_DETAIL_BUFFER_HPP
