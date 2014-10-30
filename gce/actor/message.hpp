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
#include <gce/actor/duration.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/service_id.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/detail/spawn.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/detail/exit.hpp>
#include <gce/actor/detail/to_match.hpp>
#include <gce/detail/cow_buffer.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/array.hpp>
#include <boost/utility/string_ref.hpp>
#include <utility>
#include <iostream>

namespace gce
{
namespace detail
{
class socket;
static match_t const tag_aid_t = atom("gce_aid_t");
static match_t const tag_request_t = atom("gce_request_t");
static match_t const tag_link_t = atom("gce_link_t");
static match_t const tag_exit_t = atom("gce_exit_t");
static match_t const tag_response_t = atom("gce_resp_t");
static match_t const tag_spawn_t = atom("gce_spw_t");
static match_t const tag_spawn_ret_t = atom("gce_spw_ret_t");

typedef boost::variant<
  aid_t, request_t, resp_t, link_t, exit_t,
  fwd_link_t, fwd_exit_t, spawn_t, spawn_ret_t
  > tag_t;

typedef boost::variant<int, aid_t, request_t> relay_t;
}

class message
{
public:
  message()
    : type_(match_nil)
    , tag_offset_(u32_nil)
    , is_copy_read_size_(false)
  {
  }

  template <typename Match>
  message(Match type)
    : type_(detail::to_match(type))
    , tag_offset_(u32_nil)
    , is_copy_read_size_(false)
  {
  }

  message(byte_t const* data, std::size_t size)
    : type_(match_nil)
    , tag_offset_(u32_nil)
    , cow_(data, size)
    , is_copy_read_size_(0)
  {
  }
  
  template <typename Match>
  message(
    Match type, byte_t const* data,
    std::size_t size, boost::uint32_t tag_offset
    )
    : type_(detail::to_match(type))
    , tag_offset_(tag_offset)
    , cow_(data, size)
    , is_copy_read_size_(0)
  {
  }

  message(message const& other)
    : type_(other.type_)
    , tag_offset_(other.tag_offset_)
    , relay_(other.relay_)
    , cow_(other.cow_)
    , is_copy_read_size_(other.is_copy_read_size_)
  {
    if (is_copy_read_size_)
    {
      cow_.data().read(other.cow_.data().read_size());
    }
  }

  message& operator=(message const& rhs)
  {
    if (this != &rhs)
    {
      type_ = rhs.type_;
      tag_offset_ = rhs.tag_offset_;
      relay_ = rhs.relay_;
      is_copy_read_size_ = rhs.is_copy_read_size_;
      cow_ = rhs.cow_;
      if (is_copy_read_size_)
      {
        cow_.data().read(rhs.cow_.data().read_size());
      }
    }
    return *this;
  }

  byte_t const* data() const
  {
    return cow_.data().data();
  }

  std::size_t size() const { return cow_.data().write_size(); }
  match_t get_type() const { return type_; }
#ifdef GCE_LUA
  match_type get_match_type() const { return match_type(type_); }
#endif
  boost::uint32_t get_tag_offset() const { return tag_offset_; }

  template <typename Match>
  void set_type(Match type) { type_ = detail::to_match(type); }
#ifdef GCE_LUA
  void set_match_type(match_type type) { type_ = type; }
#endif
  void reset_write() { cow_.data().clear_write(); }
  void reset_read() { cow_.data().clear_read(); }

  template <typename T>
  message& operator<<(T const& t)
  {
    boost::amsg::error_code_t ec = boost::amsg::success;
    std::size_t size = boost::amsg::size_of(t, ec);
    if (ec != boost::amsg::success)
    {
      boost::amsg::base_store bs;
      bs.set_error_code(ec);
      GCE_VERIFY(false)(size)(ec).msg(bs.message());
    }

    cow_.reserve(size);
    detail::buffer_ref& buf = cow_.data();

    boost::amsg::zero_copy_buffer writer(
      buf.get_write_data(), buf.remain_write_size()
      );

    boost::amsg::write(writer, t);
    BOOST_ASSERT(!writer.bad());

    buf.write(writer.write_length());
    return *this;
  }

  template <typename T>
  message& operator>>(T& t)
  {
    detail::buffer_ref& buf = cow_.data();
    boost::amsg::zero_copy_buffer reader(
      buf.get_read_data(), buf.remain_read_size()
      );

    boost::amsg::read(reader, t);
    GCE_VERIFY(!reader.bad())(buf.remain_read_size()).msg("read data overflow");

    buf.read(reader.read_length());
    return *this;
  }

  message& operator<<(message const m)
  {
    boost::uint32_t msg_size = (boost::uint32_t)m.size();
    match_t msg_type = m.get_type();
    boost::uint32_t tag_offset = m.tag_offset_;
    detail::buffer_ref& buf = cow_.data();

    boost::amsg::error_code_t ec = boost::amsg::success;
    std::size_t size = boost::amsg::size_of(msg_size, ec);
    size += boost::amsg::size_of(msg_type, ec);
    size += boost::amsg::size_of(tag_offset, ec);
    size += msg_size;
    if (ec != boost::amsg::success)
    {
      boost::amsg::base_store bs;
      bs.set_error_code(ec);
      GCE_VERIFY(false)(msg_size)(msg_type)(size)(ec).msg(bs.message());
    }

    cow_.reserve(size);

    boost::amsg::zero_copy_buffer writer(
      buf.get_write_data(), buf.remain_write_size()
      );

    boost::amsg::write(writer, msg_size);
    BOOST_ASSERT(!writer.bad());
    boost::amsg::write(writer, msg_type);
    BOOST_ASSERT(!writer.bad());
    boost::amsg::write(writer, tag_offset);
    BOOST_ASSERT(!writer.bad());

    buf.write(writer.write_length());
    byte_t* write_data = buf.get_write_data();
    buf.write(msg_size);
    std::memcpy(write_data, m.data(), msg_size);
    return *this;
  }

  message& operator>>(message& msg)
  {
    boost::uint32_t msg_size;
    match_t msg_type;
    boost::uint32_t tag_offset;
    detail::buffer_ref& buf = cow_.data();
    boost::amsg::zero_copy_buffer reader(
      buf.get_read_data(), buf.remain_read_size()
      );

    boost::amsg::read(reader, msg_size);
    GCE_VERIFY(!reader.bad())(msg_size).msg("read data overflow");

    boost::amsg::read(reader, msg_type);
    GCE_VERIFY(!reader.bad())(msg_type).msg("read data overflow");

    boost::amsg::read(reader, tag_offset);
    GCE_VERIFY(!reader.bad())(tag_offset).msg("read data overflow");

    buf.read(reader.read_length());
    byte_t* read_data = buf.get_read_data();
    buf.read(msg_size);
    msg = message(msg_type, read_data, msg_size, tag_offset);
    return *this;
  }

  message& operator<<(boost::string_ref str)
  {
    boost::uint32_t size = (boost::uint32_t)str.size();
    *this << size;
    cow_.append(str.data(), size);
    return *this;
  }

  message& operator<<(char const* str)
  {
    boost::uint32_t size = (boost::uint32_t)std::char_traits<char>::length(str);
    *this << size;
    cow_.append(str, size);
    return *this;
  }

  message& operator>>(boost::string_ref& str)
  {
    boost::uint32_t size;
    *this >> size;
    detail::buffer_ref& buf = cow_.data();
    str = boost::string_ref((char const*)buf.get_read_data(), size);
    buf.read(size);
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

#ifdef GCE_LUA
  message& operator<<(duration_type const& dur)
  {
    *this << dur.dur_.count();
    return *this;
  }

  message& operator>>(duration_type& dur)
  {
    duration_t::rep c;
    *this >> c;
    dur = duration_type(duration_t(c));
    return *this;
  }
#endif

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

  std::string to_string() const
  {
    std::string rt;
    rt += "<";
    rt += gce::atom(get_type());
    rt += ".";
    rt += boost::lexical_cast<std::string>(size());
    rt += ">";
    return rt;
  }

#ifdef GCE_LUA
  int get_overloading_type() const
  {
    return (int)detail::overloading_msg;
  }

  GCE_LUA_SERIALIZE_FUNC
#endif

public:
  void push_tag(
    detail::tag_t& tag, aid_t recver,
    svcid_t svc, aid_t skt, bool is_err_ret
    )
  {
    tag_offset_ = (boost::uint32_t)cow_.data().write_size();
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
    else if (resp_t* res = boost::get<resp_t>(&tag))
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
      detail::buffer_ref& buf = cow_.data();
      BOOST_ASSERT(tag_offset_ < buf.write_size());
      buf.read(tag_offset_);
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
        tag = resp_t(id, aid);
      }
      else if (tag_type == detail::tag_spawn_t)
      {
        boost::uint16_t type;
        std::string func;
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

      buf.clear();
      buf.write(tag_offset_);
    }
    return has_tag;
  }

  void enable_copy_read_size()
  {
    is_copy_read_size_ = true;
  }

  void disable_copy_read_size()
  {
    is_copy_read_size_ = false;
  }

  template <typename T>
  T const* get_relay() const
  {
    return boost::get<T>(&relay_);
  }

  template <typename T>
  void set_relay(T const& relay)
  {
    relay_ = relay;
  }

  void clear_relay()
  {
    relay_ = detail::relay_t();
  }

private:
  match_t type_;
  boost::uint32_t tag_offset_;
  detail::cow_buffer<GCE_SMALL_MSG_SIZE, GCE_MSG_MIN_GROW_SIZE> cow_;

  detail::relay_t relay_;
  bool is_copy_read_size_;
};

typedef message msg_t;
#ifdef GCE_LUA
msg_t lua_msg()
{
  return msg_t();
}
#endif
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
