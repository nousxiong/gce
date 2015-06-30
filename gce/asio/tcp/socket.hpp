///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_TCP_SOCKET_HPP
#define GCE_ASIO_TCP_SOCKET_HPP

#include <gce/asio/config.hpp>
#include <boost/optional.hpp>
#include <boost/array.hpp>

namespace gce
{
namespace asio
{
namespace tcp
{
static match_t const as_conn = atom("as_conn");
static match_t const as_recv = atom("as_recv");
static match_t const as_recv_some = atom("as_recv_some");
static match_t const as_recv_until = atom("as_recv_until");
static match_t const as_send = atom("as_send");
static match_t const as_send_some = atom("as_send_some");

/// a wrapper for boost::asio::ip::tcp::socket
class socket
  : public addon_t
{
  enum
  {
    ha_conn = 0,
    ha_recv,
    ha_send,

    ha_num,
  };

  typedef addon_t base_t;
  typedef socket self_t;
  typedef base_t::scope<self_t, boost::array<gce::detail::handler_allocator_t, ha_num> > scope_t;
  typedef scope_t::guard_ptr guard_ptr;

public:
  typedef boost::asio::ip::tcp::socket impl_t;
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef boost::asio::ip::tcp::socket tcp_socket_t;

public:
  template <typename Actor>
  explicit socket(Actor a)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , skt_opt_(boost::in_place(boost::ref(snd_.get_io_service())))
    , impl_(&skt_opt_.get())
    , conning_(false)
    , recving_(false)
    , sending_(false)
    , scp_(this)
  {
  }
  
  template <typename Actor>
  explicit socket(Actor a, boost::shared_ptr<tcp_socket_t> skt)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , skt_ptr_(skt)
    , impl_(skt_ptr_.get())
    , conning_(false)
    , recving_(false)
    , sending_(false)
    , scp_(this)
  {
  }

  ~socket()
  {
  }
  
public:
  /// connect directly (no resolve)
  void async_connect(impl_t::endpoint_type const& ep, message const& msg = message(as_conn))
  {
    GCE_ASSERT(!conning_);
    GCE_ASSERT(!recving_);
    GCE_ASSERT(!sending_);

    conn_msg_ = msg;
    impl_->async_connect(
      ep, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_conn],
          boost::bind(
            &self_t::handle_connect, scp_.get(),
            boost::asio::placeholders::error
            )
          )
        )
      );
    conning_ = true;
  }
  
  /// resovlve before connect
  void async_connect(resolver_t::iterator itr, message const& msg = message(as_conn))
  {
    GCE_ASSERT(!conning_);
    GCE_ASSERT(!recving_);
    GCE_ASSERT(!sending_);

    conn_msg_ = msg;
    boost::asio::async_connect(
      *impl_, itr,
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_conn],
          boost::bind(
            &self_t::handle_connect, scp_.get(),
            boost::asio::placeholders::error
            )
          )
        )
      );
    conning_ = true;
  }
  
  template <typename MutableBufferSequence>
  void async_read(MutableBufferSequence const& buffers, message const& msg = message(as_recv))
  {
    GCE_ASSERT(!recving_);
    GCE_ASSERT(!conning_);
    
    recv_msg_ = msg;
    boost::asio::async_read(
      *impl_, buffers, 
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
    GCE_ASSERT(!conning_);
    recv_msg_ = msg;
    message& m = recv_msg_;

    m.to_large();
    m << errcode_t();

    m.pre_write(length);
    byte_t* buffer = m.reset_write(0);

    boost::asio::async_read(
      *impl_, boost::asio::buffer(buffer, length), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_recv],
          boost::bind(
            &self_t::handle_recv_length, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
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
    GCE_ASSERT(!conning_);
    
    recv_msg_ = msg;
    impl_->async_read_some(
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
    GCE_ASSERT(!conning_);
    recv_msg_ = msg;
    message& m = recv_msg_;

    m.to_large();
    m << errcode_t();

    m.pre_write(length);
    byte_t* buffer = m.reset_write(0);
    
    impl_->async_read_some(
      boost::asio::buffer(buffer, length), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_recv],
          boost::bind(
            &self_t::handle_recv_length, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
    recving_ = true;
  }

  template <typename Allocator, typename Expr>
  void async_read_until(
    boost::asio::basic_streambuf<Allocator>& b, 
    Expr const& expr, 
    message const& msg = message(as_recv_until)
    )
  {
    GCE_ASSERT(!recving_);
    GCE_ASSERT(!conning_);

    recv_msg_ = msg;
    boost::asio::async_read_until(
      *impl_, b, expr,
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

  template <typename ConstBufferSequence>
  void async_write(ConstBufferSequence const& buffers, message const& msg = message(as_send))
  {
    GCE_ASSERT(!sending_);
    GCE_ASSERT(!conning_);
    
    send_msg_ = msg;
    boost::asio::async_write(
      *impl_, buffers, 
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
    GCE_ASSERT(!conning_);

    send_msg_ = msg;
    message& m = send_msg_;

    message::chunk ch(length);
    m >> ch;
    boost::asio::async_write(
      *impl_, boost::asio::buffer(ch.data(), ch.size()), 
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
    GCE_ASSERT(!conning_);

    send_msg_ = msg;
    impl_->async_write_some(
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
    GCE_ASSERT(!conning_);

    send_msg_ = msg;
    message& m = send_msg_;
    message::chunk ch(length);
    m >> ch;
    impl_->async_write_some(
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
    return impl_;
  }

  void dispose()
  {
    scp_.notify();
    errcode_t ignored_ec;
    impl_->close(ignored_ec);
  }
  
  impl_t& get_impl()
  {
    return *impl_;
  }

private:
  static void handle_connect(guard_ptr guard, errcode_t const& ec)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }
    
    o->conning_ = false;
    message& m = o->conn_msg_;
    m << ec;
    o->pri_send2actor(m);
  }

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
  
  static void handle_recv_length(guard_ptr guard, errcode_t const& ec, size_t bytes_transferred)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->recving_ = false;

    message& m = o->recv_msg_;
    packer& pkr = m.get_packer();
    pkr.skip_write(bytes_transferred);
    m.end_write();

    m.reset_write(packer::size_of(gce::adl::detail::errcode()) + bytes_transferred);
    m << ec << message::skip(bytes_transferred);

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
    base_t::send2actor(msg);
  }

private:
  strand_t& snd_;
  boost::optional<impl_t> skt_opt_;
  boost::shared_ptr<impl_t> skt_ptr_;
  impl_t* impl_;
  bool conning_;
  bool recving_;
  bool sending_;
  
  message conn_msg_;
  message recv_msg_;
  message send_msg_;
  message const msg_nil_;

  /// for quit
  scope_t scp_;
};
} /// namespace tcp
} /// namespace asio
} /// namespace gce

#endif /// GCE_ASIO_TCP_SOCKET_HPP
