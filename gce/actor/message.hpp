///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_MESSAGE_HPP
#define GCE_ACTOR_MESSAGE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/buffer.hpp>
#include <gce/actor/detail/buffer_ref.hpp>
#include <gce/amsg/amsg.hpp>
#include <gce/amsg/zerocopy.hpp>
#include <boost/call_traits.hpp>

namespace gce
{
class message
{
public:
  message()
    : type_(match_nil)
    , buf_(small_, GCE_SMALL_MSG_SIZE)
  {
  }

  message(match_t type)
    : type_(type)
    , buf_(small_, GCE_SMALL_MSG_SIZE)
  {
  }

  message(match_t type, byte_t const* data, std::size_t size)
    : type_(type)
  {
    if (size <= GCE_SMALL_MSG_SIZE)
    {
      buf_.reset(small_, GCE_SMALL_MSG_SIZE);
    }
    else
    {
      make_large(size);
      buf_.reset(large_->data(), size);
    }
    std::memcpy(buf_.get_write_data(), data, size);
    buf_.write(size);
  }

  message(message const& other)
    : type_(other.type_)
  {
    detail::buffer_ref const& buf = other.buf_;
    large_ = other.large_;
    if (other.is_small())
    {
      buf_.reset(small_, GCE_SMALL_MSG_SIZE);
      std::memcpy(small_, other.small_, buf.write_size());
    }
    else
    {
      BOOST_ASSERT(large_);
      buf_.reset(large_->data(), large_->size());
    }
    buf_.write(buf.write_size());
    //buf_.read(buf.read_size());
  }

  message& operator=(message const& rhs)
  {
    if (this != &rhs)
    {
      type_ = rhs.type_;
      detail::buffer_ref const& buf = rhs.buf_;
      buf_.clear();

      if (rhs.is_small())
      {
        if (!is_small())
        {
          large_.reset();
        }

        buf_.reset(small_, GCE_SMALL_MSG_SIZE);
        std::memcpy(small_, rhs.small_, buf.write_size());
      }
      else
      {
        BOOST_ASSERT(rhs.large_);
        large_ = rhs.large_;
        buf_.reset(large_->data(), large_->size());
      }
      buf_.write(buf.write_size());
      //buf_.read(buf.read_size());
    }
    return *this;
  }

  inline byte_t const* data() const
  {
    return buf_.data();
  }

  inline std::size_t size() const { return buf_.write_size(); }
  inline match_t get_type() const { return type_; }

  template <typename T>
  message& operator<<(T const& t)
  {
    boost::amsg::error_code_t ec = boost::amsg::success;
    std::size_t size = boost::amsg::size_of(t, ec);
    if (ec != boost::amsg::success)
    {
      boost::amsg::base_store bs;
      bs.set_error_code(ec);
      throw std::runtime_error(bs.message());
    }

    if (size > buf_.remain_write_size())
    {
      std::cout << "resize buffer\n";
      std::size_t old_buf_size = buf_.size();
      std::size_t new_buf_size = buf_.write_size() + size;
      if (new_buf_size - old_buf_size < GCE_MSG_MIN_GROW_SIZE)
      {
        new_buf_size = old_buf_size + GCE_MSG_MIN_GROW_SIZE;
      }

      if (is_small())
      {
        make_large(new_buf_size);
        std::memcpy(large_->data(), buf_.data(), buf_.write_size());
      }
      else
      {
        BOOST_ASSERT(large_);
        /// copy-on-write
        if (large_->use_count() > 1)
        {
          std::cout << "copy-on-write\n";
          detail::buffer_ptr tmp = large_;
          make_large(new_buf_size);
          std::memcpy(large_->data(), tmp->data(), old_buf_size);
        }
        else
        {
          large_->resize(new_buf_size);
        }
      }
      buf_.reset(large_->data(), new_buf_size);
    }

    boost::amsg::zero_copy_buffer writer(
      buf_.get_write_data(), buf_.remain_write_size()
      );

    boost::amsg::write(writer, t);
    BOOST_ASSERT(!writer.bad());

    buf_.write(writer.write_length());
    return *this;
  }

  template <typename T>
  message& operator>>(T& t)
  {
    boost::amsg::zero_copy_buffer reader(
      buf_.get_read_data(), buf_.remain_read_size()
      );

    boost::amsg::read(reader, t);
    if (reader.bad())
    {
      throw std::runtime_error("read data overflow");
    }

    buf_.read(reader.read_length());
    return *this;
  }

  inline bool is_small() const
  {
    return buf_.data() == small_;
  }

private:
  inline void make_large(std::size_t size)
  {
    large_.reset(GCE_CACHE_ALIGNED_NEW(detail::buffer)(size));
  }

private:
  match_t type_;
  byte_t small_[GCE_SMALL_MSG_SIZE];
  detail::buffer_ptr large_;
  detail::buffer_ref buf_;
};
}

#endif /// GCE_ACTOR_MESSAGE_HPP
