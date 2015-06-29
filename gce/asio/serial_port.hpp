///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_SERIAL_PORT_HPP
#define GCE_ASIO_SERIAL_PORT_HPP

#include <gce/asio/config.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>

namespace gce
{
namespace asio
{
static match_t const as_recv = atom("as_recv");
static match_t const as_recv_some = atom("as_recv_some");
static match_t const as_send = atom("as_send");
static match_t const as_send_some = atom("as_send_some");

/// a wrapper for boost::asio::serial_port
class serial_port
  : public addon_t
{
  enum
  {
    ha_recv = 0,
    ha_send,

    ha_num,
  };

  typedef addon_t base_t;
  typedef serial_port self_t;
  typedef base_t::scope<self_t, boost::array<gce::detail::handler_allocator_t, ha_num> > scope_t;
  typedef scope_t::guard_ptr guard_ptr;

public:
  typedef boost::asio::serial_port impl_t;

public:
  template <typename Actor>
  explicit serial_port(Actor a)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service())
    , recving_(false)
    , sending_(false)
    , scp_(this)
  {
  }

  template <typename Actor>
  explicit serial_port(Actor a, std::string const& device)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service(), device)
    , recving_(false)
    , sending_(false)
    , scp_(this)
  {
  }

public:
  template <typename MutableBufferSequence>
  void async_read(MutableBufferSequence const& buffers, message const& msg = message(as_recv))
  {
    GCE_ASSERT(!recving_);
    
    recv_msg_ = msg;
    boost::asio::async_read(
      impl_, buffers, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_recv],
          boost::bind(
            &self_t::handle_recv, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
    recving_ = true;
  }
  
  void async_read(size_t length, message const& msg = message(as_recv))
  {
    GCE_ASSERT(!recving_);
    recv_msg_ = msg;
    message& m = recv_msg_;

    m.to_large();
    m << errcode_t();

    m.pre_write(length);
    byte_t* buffer = m.reset_write(0);

    boost::asio::async_read(
      impl_, boost::asio::buffer(buffer, length), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_recv],
          boost::bind(
            &self_t::handle_recv_length, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, length
            )
          )
        )
      );
    recving_ = true;
  }

  template <typename MutableBufferSequence>
  void async_read_some(MutableBufferSequence const& buffers, message const& msg = message(as_recv_some))
  {
    GCE_ASSERT(!recving_);
    
    recv_msg_ = msg;
    impl_.async_read_some(
      buffers, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_recv],
          boost::bind(
            &self_t::handle_recv, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
    recving_ = true;
  }
  
  void async_read_some(size_t length, message const& msg = message(as_recv_some))
  {
    GCE_ASSERT(!recving_);
    recv_msg_ = msg;
    message& m = recv_msg_;

    m.to_large();
    m << errcode_t();

    m.pre_write(length);
    byte_t* buffer = m.reset_write(0);
    
    impl_.async_read_some(
      boost::asio::buffer(buffer, length), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_recv],
          boost::bind(
            &self_t::handle_recv_length, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, length
            )
          )
        )
      );
    recving_ = true;
  }
  
  template <typename ConstBufferSequence>
  void async_write(ConstBufferSequence const& buffers, message const& msg = message(as_send))
  {
    GCE_ASSERT(!sending_);
    
    send_msg_ = msg;
    boost::asio::async_write(
      impl_, buffers, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_send],
          boost::bind(
            &self_t::handle_send, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
    sending_ = true;
  }
  
  void async_write(message const& msg, size_t length = size_nil)
  {
    GCE_ASSERT(!sending_);

    send_msg_ = msg;
    message& m = send_msg_;

    message::chunk ch(length);
    m >> ch;
    boost::asio::async_write(
      impl_, boost::asio::buffer(ch.data(), ch.size()), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_send],
          boost::bind(
            &self_t::handle_send, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
    sending_ = true;
  }
  
  template <typename ConstBufferSequence>
  void async_write_some(ConstBufferSequence const& buffers, message const& msg = message(as_send_some))
  {
    GCE_ASSERT(!sending_);

    send_msg_ = msg;
    impl_.async_write_some(
      buffers, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_send],
          boost::bind(
            &self_t::handle_send, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
    sending_ = true;
  }
  
  void async_write_some(message const& msg, size_t offset = 0, size_t length = size_nil)
  {
    GCE_ASSERT(!sending_);

    send_msg_ = msg;
    message& m = send_msg_;
    message::chunk ch(length);
    m >> ch;
    impl_.async_write_some(
      boost::asio::buffer(ch.data()+offset, ch.size()), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_send],
          boost::bind(
            &self_t::handle_send, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
    sending_ = true;
  }

  /// for using other none-async methods directly
  impl_t* operator->()
  {
    return &impl_;
  }

  void dispose()
  {
    scp_.notify();
    errcode_t ignored_ec;
    impl_.close(ignored_ec);
  }

  impl_t& get_impl()
  {
    return impl_;
  }

private:
  static void handle_recv(guard_ptr guard, errcode_t const& ec, size_t bytes_transferred)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }
    
    o->recving_ = false;
    message& m = o->recv_msg_;
    m << ec << bytes_transferred;
    o->pri_send2actor(m);
  }
  
  static void handle_recv_length(guard_ptr guard, errcode_t const& ec, size_t bytes_transferred, size_t length)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->recving_ = false;

    message& m = o->recv_msg_;
    packer& pkr = m.get_packer();
    pkr.skip_write(length);
    m.end_write();

    m.reset_write(packer::size_of(gce::adl::detail::errcode()) + length);
    m << ec << message::skip(length) << bytes_transferred;

    o->pri_send2actor(m);
  }
  
  static void handle_send(guard_ptr guard, errcode_t const& ec, size_t bytes_transferred)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }
    
    o->sending_ = false;
    message& m = o->send_msg_;
    m << ec << bytes_transferred;
    o->pri_send2actor(m);
  }
  
private:
  void pri_send2actor(message& m)
  {
    message msg(m);
    m = msg_nil_;
    send2actor(msg);
  }

private:
  strand_t& snd_;
  impl_t impl_;
  bool recving_;
  bool sending_;

  message recv_msg_;
  message send_msg_;
  message const msg_nil_;

  /// for quit
  scope_t scp_;
};
}
}

#endif /// GCE_ASIO_SERIAL_PORT_HPP
