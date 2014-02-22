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

    reserve(size);

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

  message& operator<<(message const m)
  {
    boost::uint32_t msg_size = m.size();
    match_t msg_type = m.get_type();
    boost::amsg::error_code_t ec = boost::amsg::success;
    std::size_t size = boost::amsg::size_of(msg_size, ec);
    size += boost::amsg::size_of(msg_type, ec);
    size += msg_size;
    if (ec != boost::amsg::success)
    {
      boost::amsg::base_store bs;
      bs.set_error_code(ec);
      throw std::runtime_error(bs.message());
    }

    reserve(size);

    boost::amsg::zero_copy_buffer writer(
      buf_.get_write_data(), buf_.remain_write_size()
      );

    boost::amsg::write(writer, msg_size);
    BOOST_ASSERT(!writer.bad());
    boost::amsg::write(writer, msg_type);
    BOOST_ASSERT(!writer.bad());

    buf_.write(writer.write_length());
    byte_t* write_data = buf_.get_write_data();
    buf_.write(msg_size);
    std::memcpy(write_data, m.data(), msg_size);
    return *this;
  }

  message& operator>>(message& msg)
  {
    boost::uint32_t msg_size;
    match_t msg_type;
    boost::amsg::zero_copy_buffer reader(
      buf_.get_read_data(), buf_.remain_read_size()
      );

    boost::amsg::read(reader, msg_size);
    if (reader.bad())
    {
      throw std::runtime_error("read data overflow");
    }

    boost::amsg::read(reader, msg_type);
    if (reader.bad())
    {
      throw std::runtime_error("read data overflow");
    }

    buf_.read(reader.read_length());
    byte_t* read_data = buf_.get_read_data();
    buf_.read(msg_size);
    msg = message(msg_type, read_data, msg_size);
    return *this;
  }

  message& operator<<(errcode_t const& ec)
  {
    boost::int32_t code = (boost::int32_t)ec.value();
    uintptr_t errcat = (uintptr_t)(&ec.category());
    *this << code << errcat;
    return *this;
  }

  message& operator>>(errcode_t& ec)
  {
    boost::int32_t code;
    uintptr_t errcat;
    *this >> code >> errcat;
    boost::system::error_category const* errcat_ptr =
      (boost::system::error_category const*)errcat;
    ec = errcode_t((int)code, *errcat_ptr);
    return *this;
  }

  template <typename Rep, typename Period>
  message& operator<<(boost::chrono::duration<Rep, Period> const& dur)
  {
    *this << dur.count();
    return *this;
  }

  template <typename Rep, typename Period>
  message& operator>>(boost::chrono::duration<Rep, Period>& dur)
  {
    typedef boost::chrono::duration<Rep, Period> duration_t;
    typename duration_t::rep c;
    *this >> c;
    dur = duration_t(c);
    return *this;
  }

  template <typename Clock>
  message& operator<<(boost::chrono::time_point<Clock, typename Clock::duration> const& tp)
  {
    *this << tp.time_since_epoch();
    return *this;
  }

  template <typename Clock>
  message& operator>>(boost::chrono::time_point<Clock, typename Clock::duration>& tp)
  {
    typename Clock::duration dur;
    *this << dur;
    tp = boost::chrono::time_point<Clock, typename Clock::duration>(dur);
    return *this;
  }

  inline bool is_small() const
  {
    return buf_.data() == small_;
  }

private:
  inline void reserve(std::size_t size)
  {
    std::size_t old_buf_capacity = buf_.size();
    std::size_t old_buf_size = buf_.write_size();
    std::size_t new_buf_size = old_buf_size + size;
    std::size_t new_buf_capacity = old_buf_capacity;
    if (new_buf_size > old_buf_capacity)
    {
      std::size_t diff = new_buf_size - old_buf_capacity;
      if (diff < GCE_MSG_MIN_GROW_SIZE)
      {
        diff = GCE_MSG_MIN_GROW_SIZE;
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
