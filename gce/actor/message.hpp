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
#include <gce/actor/detail/request.hpp>
#include <gce/actor/service_id.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/detail/spawn.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/detail/exit.hpp>
#include <gce/amsg/amsg.hpp>
#include <gce/amsg/zerocopy.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/utility/string_ref.hpp>
#include <utility>
#include <iostream>

namespace gce
{
class basic_actor;
class coroutine_stackful_actor;
class thread_mapped_actor;
class coroutine_stackless_actor;
class nonblocking_actor;
namespace detail
{
class socket;
static match_t const tag_aid_t = atom("gce_aid_t");
static match_t const tag_request_t = atom("gce_request_t");
static match_t const tag_link_t = atom("gce_link_t");
static match_t const tag_exit_t = atom("gce_exit_t");
static match_t const tag_response_t = atom("gce_response_t");
static match_t const tag_spawn_t = atom("gce_spw_t");
static match_t const tag_spawn_ret_t = atom("gce_spw_ret_t");

typedef boost::variant<
  aid_t, request_t, response_t, link_t, exit_t,
  fwd_link_t, fwd_exit_t, spawn_t, spawn_ret_t
  > tag_t;
}

class message
{
public:
  message()
    : type_(match_nil)
    , tag_offset_(u32_nil)
    , buf_(small_, GCE_SMALL_MSG_SIZE)
  {
  }

  message(match_t type)
    : type_(type)
    , tag_offset_(u32_nil)
    , buf_(small_, GCE_SMALL_MSG_SIZE)
  {
  }

  message(byte_t const* data, std::size_t size)
    : type_(match_nil)
    , tag_offset_(u32_nil)
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

  message(
    match_t type, byte_t const* data,
    std::size_t size, boost::uint32_t tag_offset
    )
    : type_(type)
    , tag_offset_(tag_offset)
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
    , tag_offset_(other.tag_offset_)
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
      tag_offset_ = rhs.tag_offset_;
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
  inline boost::uint32_t get_tag_offset() const { return tag_offset_; }
  inline void set_type(match_t type) { type_ = type; }

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
    boost::uint32_t msg_size = (boost::uint32_t)m.size();
    match_t msg_type = m.get_type();
    boost::uint32_t tag_offset = m.tag_offset_;
    boost::amsg::error_code_t ec = boost::amsg::success;
    std::size_t size = boost::amsg::size_of(msg_size, ec);
    size += boost::amsg::size_of(msg_type, ec);
    size += boost::amsg::size_of(tag_offset, ec);
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
    boost::amsg::write(writer, tag_offset);
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
    boost::uint32_t tag_offset;
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

    boost::amsg::read(reader, tag_offset);
    if (reader.bad())
    {
      throw std::runtime_error("read data overflow");
    }

    buf_.read(reader.read_length());
    byte_t* read_data = buf_.get_read_data();
    buf_.read(msg_size);
    msg = message(msg_type, read_data, msg_size, tag_offset);
    return *this;
  }

  message& operator<<(boost::string_ref str)
  {
    boost::uint32_t size = (boost::uint32_t)str.size();
    *this << size;
    reserve(size);
    byte_t* write_data = buf_.get_write_data();
    buf_.write(size);
    std::memcpy(write_data, str.data(), size);
    return *this;
  }

  message& operator>>(boost::string_ref& str)
  {
    boost::uint32_t size;
    *this >> size;
    str = boost::string_ref((char const*)buf_.get_read_data(), size);
    buf_.read(size);
    return *this;
  }

  message& operator<<(errcode_t const& ec)
  {
    boost::int32_t code = (boost::int32_t)ec.value();
    boost::uint64_t errcat = (boost::uint64_t)(&ec.category());
    *this << code << errcat;
    return *this;
  }

  message& operator>>(errcode_t& ec)
  {
    boost::int32_t code;
    boost::uint64_t errcat;
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

  template <typename T, typename U>
  message& operator<<(std::pair<T, U> const& pr)
  {
    *this << pr.first << pr.second;
    return *this;
  }

  template <typename T, typename U>
  message& operator>>(std::pair<T, U>& pr)
  {
    *this >> pr.first >> pr.second;
    return *this;
  }

  message& operator<<(ctxid_pair_t const& pr)
  {
    *this << pr.first << (boost::uint16_t)pr.second;
    return *this;
  }

  message& operator>>(ctxid_pair_t& pr)
  {
    boost::uint16_t type;
    *this >> pr.first >> type;
    pr.second = (detail::socket_type)type;
    return *this;
  }

  message& operator<<(bool flag)
  {
    boost::uint16_t f = flag ? 1 : 0;
    *this << f;
    return *this;
  }

  message& operator>>(bool& flag)
  {
    boost::uint16_t f;
    *this >> f;
    flag = f != 0;
    return *this;
  }

  inline bool is_small() const
  {
    return buf_.data() == small_;
  }

private:
  void push_tag(
    detail::tag_t& tag, aid_t recver,
    svcid_t svc, aid_t skt, bool is_err_ret
    )
  {
    tag_offset_ = (boost::uint32_t)buf_.write_size();
    if (aid_t* aid = boost::get<aid_t>(&tag))
    {
      *this << detail::tag_aid_t << *aid;
    }
    else if (detail::request_t* req = boost::get<detail::request_t>(&tag))
    {
      *this << detail::tag_request_t << req->get_id() << req->get_aid();
    }
    else if (detail::link_t* link = boost::get<detail::link_t>(&tag))
    {
      *this << detail::tag_link_t <<
        (boost::uint16_t)link->get_type() << link->get_aid();
    }
    else if (detail::exit_t* ex = boost::get<detail::exit_t>(&tag))
    {
      *this << detail::tag_exit_t <<
        ex->get_code() << ex->get_aid();
    }
    else if (response_t* res = boost::get<response_t>(&tag))
    {
      *this << detail::tag_response_t <<
        res->get_id() << res->get_aid();
    }
    else if (detail::spawn_t* spw = boost::get<detail::spawn_t>(&tag))
    {
      *this << detail::tag_spawn_t << (boost::uint16_t)spw->get_type() << 
        spw->get_func() << spw->get_ctxid() << spw->get_stack_size() <<
        spw->get_id() << spw->get_aid();
    }
    else if (detail::spawn_ret_t* spr = boost::get<detail::spawn_ret_t>(&tag))
    {
      *this << detail::tag_spawn_ret_t << (boost::uint16_t)spr->get_error() <<
        spr->get_id() << spr->get_aid();
    }
    else
    {
      BOOST_ASSERT(false);
    }
    *this << recver << svc << skt << is_err_ret;
  }

  bool pop_tag(
    detail::tag_t& tag, aid_t& recver,
    svcid_t& svc, aid_t& skt, bool& is_err_ret
    )
  {
    bool has_tag = false;
    if (tag_offset_ != u32_nil)
    {
      BOOST_ASSERT(tag_offset_ < buf_.write_size());
      buf_.read(tag_offset_);
      match_t tag_type;
      has_tag = true;
      *this >> tag_type;
      if (tag_type == detail::tag_aid_t)
      {
        aid_t aid;
        *this >> aid;
        tag = aid;
      }
      else if (tag_type == detail::tag_request_t)
      {
        sid_t id;
        aid_t aid;
        *this >> id >> aid;
        tag = detail::request_t(id, aid);
      }
      else if (tag_type == detail::tag_link_t)
      {
        boost::uint16_t type;
        aid_t aid;
        *this >> type >> aid;
        tag = detail::link_t((link_type)type, aid);
      }
      else if (tag_type == detail::tag_exit_t)
      {
        exit_code_t ec;
        aid_t aid;
        *this >> ec >> aid;
        tag = detail::exit_t(ec, aid);
      }
      else if (tag_type == detail::tag_response_t)
      {
        sid_t id;
        aid_t aid;
        *this >> id >> aid;
        tag = response_t(id, aid);
      }
      else if (tag_type == detail::tag_spawn_t)
      {
        boost::uint16_t type;
        match_t func;
        match_t ctxid;
        std::size_t stack_size;
        sid_t sid;
        aid_t aid;
        *this >> type >> func >> ctxid >> stack_size >> sid >> aid;
        tag = detail::spawn_t(
          (detail::spawn_type)type, func, ctxid, stack_size, sid, aid
          );
      }
      else if (tag_type == detail::tag_spawn_ret_t)
      {
        boost::uint16_t err;
        sid_t sid;
        aid_t aid;
        *this >> err >> sid >> aid;
        tag = detail::spawn_ret_t((detail::spawn_error)err, sid, aid);
      }
      else
      {
        BOOST_ASSERT(false);
      }
      *this >> recver >> svc >> skt >> is_err_ret;

      buf_.clear();
      buf_.write(tag_offset_);
    }
    return has_tag;
  }

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
    large_.reset(new detail::buffer(size));
  }

private:
  match_t type_;
  boost::uint32_t tag_offset_;
  byte_t small_[GCE_SMALL_MSG_SIZE];
  detail::buffer_ptr large_;
  detail::buffer_ref buf_;

  friend class basic_actor;
  friend class coroutine_stackful_actor;
  friend class thread_mapped_actor;
  friend class coroutine_stackless_actor;
  friend class nonblocking_actor;
  friend class detail::socket;
  detail::request_t req_;
};
}

template<typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(
  std::basic_ostream<CharT, TraitsT>& strm, gce::message const& msg
  )
{
  strm << "<" << gce::atom(msg.get_type()) << "." << msg.size() << ">";
  return strm;
}

#endif /// GCE_ACTOR_MESSAGE_HPP
