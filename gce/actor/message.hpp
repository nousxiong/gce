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
#include <gce/actor/atom.hpp>
#include <gce/actor/error_code.hpp>
#include <gce/actor/detail/internal.hpp>
#include <gce/actor/packer.hpp>
#include <gce/actor/detail/spawn.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/detail/exit.hpp>
#include <gce/actor/to_match.hpp>
#include <gce/detail/cow_buffer.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/get.hpp>
#include <boost/array.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/foreach.hpp>
#include <utility>
#include <iostream>

#ifndef GCE_SMALL_MSG_SIZE
# define GCE_SMALL_MSG_SIZE 128
#endif

#ifndef GCE_MSG_MIN_GROW_SIZE
# define GCE_MSG_MIN_GROW_SIZE 64
#endif

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
  struct chunk
  {
    explicit chunk(size_t len = size_nil)
      : data_(0)
      , len_(len)
    {
    }

    chunk(byte_t const* data, size_t len)
      : data_(data)
      , len_(len)
    {
    }

    byte_t const* data() const
    {
      return data_;
    }

    size_t size() const
    {
      return len_;
    }

    byte_t const* data_;
    size_t len_;
  };

  struct skip
  {
    template <typename T>
    explicit skip(T const& t)
      : len_(packer::size_of(t))
    {
    }

    explicit skip(size_t len)
      : len_(len)
    {
    }

    size_t len_;
  };

  typedef detail::header_t header;

  static header make_header(
    uint32_t size = 0, 
    match_t type = match_nil, 
    uint32_t tag_offset = u32_nil
    )
  {
    return detail::make_header(size, type, tag_offset);
  }

public:
  message()
    : tag_offset_(u32_nil)
  {
  }

  template <typename Match>
  message(Match type)
    : type_(to_match(type))
    , tag_offset_(u32_nil)
  {
  }

  template <typename Match>
  message(Match type, uint32_t tag_offset)
    : type_(to_match(type))
    , tag_offset_(tag_offset)
  {
  }

  template <typename Match>
  message(Match type, byte_t const* data, size_t size, uint32_t tag_offset)
    : type_(to_match(type))
    , tag_offset_(tag_offset)
    , cow_(data, size)
  {
  }

  message(message const& other)
    : type_(other.type_)
    , tag_offset_(other.tag_offset_)
    , cow_(other.cow_)
    , pkr_(other.pkr_)
    , relay_(other.relay_)
    , shared_list_(other.shared_list_)
  {
  }

  message& operator=(message const& rhs)
  {
    if (this != &rhs)
    {
      type_ = rhs.type_;
      tag_offset_ = rhs.tag_offset_;
      cow_ = rhs.cow_;
      pkr_ = rhs.pkr_;
      relay_ = rhs.relay_;
      shared_list_ = rhs.shared_list_;
    }
    return *this;
  }

  byte_t const* data() const
  {
    return cow_.get_buffer_ref().data();
  }

  size_t size() const { return cow_.get_buffer_ref().write_size(); }
  match_t get_type() const { return type_; }
  uint32_t get_tag_offset() const { return tag_offset_; }
  size_t shared_size() const { return shared_list_.size(); }

  template <typename Match>
  void set_type(Match type)
  {
    type_ = to_match(type);
  }

  byte_t const* reset_read(size_t len = size_nil)
  {
    return cow_.get_buffer_ref().clear_read(len);
  }

  byte_t* reset_write(size_t len = size_nil)
  {
    return cow_.get_buffer_ref().clear_write(len);
  }

  void to_large(size_t size = GCE_SMALL_MSG_SIZE + 8)
  {
    if (cow_.is_small())
    {
      cow_.reserve(size);
    }
  }

  template <typename T>
  message& operator<<(T const& t)
  {
    size_t size = packer::size_of(t);
    pre_write(size);
    pkr_.write(t);
    end_write();
    return *this;
  }

  template <typename T>
  message& operator>>(T& t)
  {
    pre_read();
    pkr_.read(t);
    end_read();
    return *this;
  }

  message& operator<<(skip s)
  {
    size_t size = s.len_;
    pre_write(size);
    pkr_.skip_write(size);
    end_write();
    return *this;
  }

  message& operator>>(skip s)
  {
    pre_read();
    pkr_.skip_read(s.len_);
    end_read();
    return *this;
  }

  template <typename T>
  message& operator<<(boost::shared_ptr<T> const& p)
  {
    uint32_t index = (uint32_t)shared_list_.size();
    size_t size = packer::size_of(index);
    pre_write(size);
    pkr_.write(index);
    shared_list_.push_back(p);
    end_write();
    return *this;
  }

  template <typename T>
  message& operator>>(boost::shared_ptr<T>& p)
  {
    uint32_t index;

    pre_read();
    pkr_.read(index);
    end_read();

    GCE_VERIFY(index < shared_list_.size())(index);
    GCE_ASSERT(shared_list_[index])(index);

    p = boost::static_pointer_cast<T>(shared_list_[index]);
    shared_list_[index].reset();
    if (index + 1 == shared_list_.size())
    {
      shared_list_.clear();
    }
    return *this;
  }

  message& operator<<(message const m) /// for self serialize, copy first
  {
    detail::header_t hdr = detail::make_header((uint32_t)m.size(), m.get_type(), m.tag_offset_);
    uint32_t shared_offset = (uint32_t)shared_list_.size();
    uint32_t shared_size = (uint32_t)m.shared_list_.size();

    size_t size = packer::size_of(hdr);
    size += hdr.size_;
    size += packer::size_of(shared_offset);
    size += packer::size_of(shared_size);

    pre_write(size);
    pkr_.write(hdr);
    pkr_.write(m.data(), hdr.size_);
    pkr_.write(shared_offset);
    pkr_.write(shared_size);
    end_write();

    shared_list_.resize(shared_offset + shared_size);
    std::copy_backward(m.shared_list_.begin(), m.shared_list_.end(), shared_list_.end());
    return *this;
  }

  message& operator>>(message& msg)
  {
    detail::header_t hdr;
    uint32_t shared_offset;
    uint32_t shared_size;

    pre_read();
    pkr_.read(hdr);
    byte_t const* body = pkr_.skip_read(hdr.size_);
    pkr_.read(shared_offset);
    pkr_.read(shared_size);
    end_read();

    message m(hdr.type_, body, hdr.size_, hdr.tag_offset_);
    m.shared_list_.resize(shared_size);
    std::copy(
      shared_list_.begin()+shared_offset, 
      shared_list_.begin()+(shared_offset+shared_size), 
      m.shared_list_.begin()
      );

    msg = m;
    return *this;
  }

  message& operator<<(std::string const& str)
  {
    uint32_t len = (uint32_t)str.size();
    size_t size = packer::size_of(len);
    size += len;

    pre_write(size);
    pkr_.write(len);
    pkr_.write((byte_t const*)str.data(), (size_t)len);
    end_write();
    return *this;
  }

  message& operator<<(boost::string_ref str)
  {
    uint32_t len = (uint32_t)str.size();
    size_t size = packer::size_of(len);
    size += len;

    pre_write(size);
    pkr_.write(len);
    pkr_.write((byte_t const*)str.data(), (size_t)len);
    end_write();
    return *this;
  }

  message& operator<<(char const* str)
  {
    uint32_t len = (uint32_t)std::char_traits<char>::length(str);
    size_t size = packer::size_of(len);
    size += len;

    pre_write(size);
    pkr_.write(len);
    pkr_.write((byte_t const*)str, (size_t)len);
    end_write();
    return *this;
  }

  message& operator>>(std::string& str)
  {
    uint32_t len = 0;
    pre_read();
    pkr_.read(len);
    byte_t const* ptr = pkr_.skip_read(len);
    str.assign((char const*)ptr, len);
    end_read();
    return *this;
  }

  message& operator>>(boost::string_ref& str)
  {
    uint32_t len = 0;
    pre_read();
    pkr_.read(len);
    byte_t const* ptr = pkr_.skip_read(len);
    str = boost::string_ref((char const*)ptr, len);
    end_read();
    return *this;
  }

  message& operator<<(errcode_t const& ec)
  {
    int32_t code = (int32_t)ec.value();
    uint64_t errcat = (uint64_t)(&ec.category());
    gce::adl::detail::errcode e = detail::make_errcode(code, errcat);
    *this << e;
    return *this;
  }

  message& operator>>(errcode_t& ec)
  {
    gce::adl::detail::errcode e;
    *this >> e;
    boost::system::error_category const* errcat_ptr =
      (boost::system::error_category const*)e.errcat_;
    ec = errcode_t((int)e.code_, *errcat_ptr);
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

  message& operator<<(duration_t const& dur)
  {
    *this << dur.val_ << dur.ty_;
    return *this;
  }

  message& operator>>(duration_t& dur)
  {
    *this >> dur.val_ >> dur.ty_;
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

  message& operator<<(detail::ctxid_pair_t const& pr)
  {
    *this << pr.first << (byte_t)pr.second;
    return *this;
  }

  message& operator>>(detail::ctxid_pair_t& pr)
  {
    byte_t type;
    *this >> pr.first >> type;
    pr.second = (detail::socket_type)type;
    return *this;
  }

  message& operator<<(bool flag)
  {
    byte_t f = flag ? 1 : 0;
    *this << f;
    return *this;
  }

  message& operator>>(bool& flag)
  {
    byte_t f;
    *this >> f;
    flag = f != 0;
    return *this;
  }

  message& operator<<(boost::asio::mutable_buffers_1 const& buffer)
  {
    uint64_t p = (uint64_t)boost::asio::buffer_cast<void*>(buffer);
    uint64_t s = (uint64_t)boost::asio::buffer_size(buffer);
    *this << p << s;
    return *this;
  }

  message& operator>>(boost::asio::mutable_buffers_1& buffer)
  {
    uint64_t p = 0;
    uint64_t s = 0;
    *this >> p >> s;
    buffer = boost::asio::buffer((void*)p, (size_t)s);
    return *this;
  }

  template <size_t N>
  message& operator<<(boost::array<boost::asio::mutable_buffers_1, N> const& buffers)
  {
    *this << (uint32_t)buffers.size();
    BOOST_FOREACH(boost::asio::mutable_buffers_1 const& buffer, buffers)
    {
      *this << buffer;
    }
    return *this;
  }

  template <size_t N>
  message& operator>>(boost::array<boost::asio::mutable_buffers_1, N>& buffers)
  {
    uint32_t size = 0;
    *this >> size;
    GCE_ASSERT(size == buffers)(size)(buffers.size());
    BOOST_FOREACH(boost::asio::mutable_buffers_1& buffer, buffers)
    {
      *this >> buffer;
    }
    return *this;
  }

  template <typename Allocator>
  message& operator<<(std::vector<boost::asio::mutable_buffers_1, Allocator> const& buffers)
  {
    *this << (uint32_t)buffers.size();
    BOOST_FOREACH(boost::asio::mutable_buffers_1 const& buffer, buffers)
    {
      *this << buffer;
    }
    return *this;
  }

  template <typename Allocator>
  message& operator>>(std::vector<boost::asio::mutable_buffers_1, Allocator>& buffers)
  {
    uint32_t size = 0;
    *this >> size;
    buffers.resize(size);
    BOOST_FOREACH(boost::asio::mutable_buffers_1& buffer, buffers)
    {
      *this >> buffer;
    }
    return *this;
  }

  message& operator<<(boost::asio::const_buffers_1 const& buffer)
  {
    uint64_t p = (uint64_t)boost::asio::buffer_cast<void const*>(buffer);
    uint64_t s = (uint64_t)boost::asio::buffer_size(buffer);
    *this << p << s;
    return *this;
  }

  message& operator>>(boost::asio::const_buffers_1& buffer)
  {
    uint64_t p = 0;
    uint64_t s = 0;
    *this >> p >> s;
    buffer = boost::asio::buffer((void const*)p, (size_t)s);
    return *this;
  }

  template <size_t N>
  message& operator<<(boost::array<boost::asio::const_buffers_1, N> const& buffers)
  {
    *this << (uint32_t)buffers.size();
    BOOST_FOREACH(boost::asio::const_buffers_1 const& buffer, buffers)
    {
      *this << buffer;
    }
    return *this;
  }

  template <size_t N>
  message& operator>>(boost::array<boost::asio::const_buffers_1, N>& buffers)
  {
    uint32_t size = 0;
    *this >> size;
    GCE_ASSERT(size == buffers)(size)(buffers.size());
    BOOST_FOREACH(boost::asio::const_buffers_1& buffer, buffers)
    {
      *this >> buffer;
    }
    return *this;
  }

  template <typename Allocator>
  message& operator<<(std::vector<boost::asio::const_buffers_1, Allocator> const& buffers)
  {
    *this << (uint32_t)buffers.size();
    BOOST_FOREACH(boost::asio::const_buffers_1 const& buffer, buffers)
    {
      *this << buffer;
    }
    return *this;
  }

  template <typename Allocator>
  message& operator>>(std::vector<boost::asio::const_buffers_1, Allocator>& buffers)
  {
    uint32_t size = 0;
    *this >> size;
    buffers.resize(size);
    BOOST_FOREACH(boost::asio::const_buffers_1& buffer, buffers)
    {
      *this >> buffer;
    }
    return *this;
  }

  message& operator<<(chunk const& ch)
  {
    GCE_ASSERT(ch.data() != 0);
    pre_write(ch.size());
    pkr_.write(ch.data(), ch.size());
    end_write();
    return *this;
  }

  message& operator>>(chunk& ch)
  {
    detail::buffer_ref& buf = cow_.get_buffer_ref();
    if (ch.len_ > buf.remain_read_size())
    {
      ch.len_ = buf.remain_read_size();
    }
    pre_read();
    ch.data_ = pkr_.skip_read(ch.len_);
    end_read();
    return *this;
  }

  message& operator>>(chunk& ch) const
  {
    detail::buffer_ref const& buf = cow_.get_buffer_ref();
    if (ch.len_ > buf.remain_read_size())
    {
      ch.len_ = buf.remain_read_size();
    }
    ch.data_ = buf.data() + buf.read_size();
    return *const_cast<message*>(this);
  }

public:
  void push_tag(
    detail::tag_t& tag, aid_t recver,
    svcid_t svc, aid_t skt, bool is_err_ret
    )
  {
    tag_offset_ = (uint32_t)cow_.get_buffer_ref().write_size();
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
        (byte_t)link->get_type() << link->get_aid();
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
      *this << detail::tag_spawn_t << (byte_t)spw->get_type() << 
        spw->get_func() << spw->get_ctxid() << spw->get_stack_size() <<
        spw->get_id() << spw->get_aid();
    }
    else if (detail::spawn_ret_t* spr = boost::get<detail::spawn_ret_t>(&tag))
    {
      *this << detail::tag_spawn_ret_t << (byte_t)spr->get_error() <<
        spr->get_id() << spr->get_aid();
    }
    else
    {
      GCE_ASSERT(false);
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
      detail::buffer_ref& buf = cow_.get_buffer_ref();
      GCE_ASSERT(tag_offset_ < buf.write_size())(tag_offset_)(buf.write_size());
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
        byte_t type;
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
        byte_t type;
        std::string func;
        match_t ctxid;
        size_t stack_size;
        sid_t sid;
        aid_t aid;
        *this >> type >> func >> ctxid >> stack_size >> sid >> aid;
        tag = detail::spawn_t(
          (detail::spawn_type)type, func, ctxid, stack_size, sid, aid
          );
      }
      else if (tag_type == detail::tag_spawn_ret_t)
      {
        byte_t err;
        sid_t sid;
        aid_t aid;
        *this >> err >> sid >> aid;
        tag = detail::spawn_ret_t((detail::spawn_error)err, sid, aid);
      }
      else
      {
        GCE_ASSERT(false);
      }
      *this >> recver >> svc >> skt >> is_err_ret;

      buf.clear();
      buf.write(tag_offset_);
    }
    return has_tag;
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

  inline packer& get_packer()
  {
    return pkr_;
  }

  inline void pre_read()
  {
    detail::buffer_ref& buf = cow_.get_buffer_ref();
    pkr_.set_read(buf.get_read_data(), buf.remain_read_size());
  }

  inline void end_read()
  {
    detail::buffer_ref& buf = cow_.get_buffer_ref();
    buf.read(pkr_.read_length());
  }

  inline void pre_write(size_t len)
  {
    cow_.reserve(len);
    detail::buffer_ref& buf = cow_.get_buffer_ref();
    pkr_.set_write(buf.get_write_data(), buf.remain_write_size());
  }

  inline void end_write()
  {
    detail::buffer_ref& buf = cow_.get_buffer_ref();
    buf.write(pkr_.write_length());
  }

private:
  match_t type_;
  uint32_t tag_offset_;
  detail::cow_buffer<GCE_SMALL_MSG_SIZE, GCE_MSG_MIN_GROW_SIZE> cow_;

  /// packer
  packer pkr_;

  /// relay helper data
  detail::relay_t relay_;

  /// local data to carry
  std::vector<boost::shared_ptr<void> > shared_list_;
};

inline std::string to_string(message const& msg)
{
  std::string str;
  str += "msg<";
  str += gce::atom(msg.get_type());
  str += ".";
  str += boost::lexical_cast<intbuf_t>(msg.size()).cbegin();
  str += ">";
  return str;
}

template <>
struct tostring<message>
{
  static std::string convert(message const& o)
  {
    return to_string(o);
  }
};

inline std::string to_string(message::chunk const& ch)
{
  std::string str;
  str += "chunk<";
  str += boost::lexical_cast<intbuf_t>(ch.data()).cbegin();
  str += ".";
  str += boost::lexical_cast<intbuf_t>(ch.size()).cbegin();
  str += ">";
  return str;
}

template <>
struct tostring<message::chunk>
{
  static std::string convert(message::chunk const& o)
  {
    return to_string(o);
  }
};
}

inline std::ostream& operator<<(std::ostream& strm, gce::message const& msg)
{
  strm << gce::to_string(msg);
  return strm;
}

inline std::ostream& operator<<(std::ostream& strm, gce::message::chunk const& ch)
{
  strm << gce::to_string(ch);
  return strm;
}

#endif /// GCE_ACTOR_MESSAGE_HPP
