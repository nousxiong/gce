///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_ACCEPTOR_HPP
#define GCE_ASIO_ACCEPTOR_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/tcp/socket.hpp>
#include <gce/asio/ssl/stream.hpp>

namespace gce
{
namespace asio
{
namespace tcp
{
static match_t const as_accept = atom("as_accept");

/// a wrapper for boost::asio::ip::tcp::acceptor
class acceptor
  : public addon_t
{
  typedef addon_t base_t;
  typedef acceptor self_t;
  typedef base_t::scope<self_t>::guard_ptr guard_ptr;

public:
  typedef boost::asio::ip::tcp::acceptor impl_t;
  typedef boost::asio::ip::tcp::socket tcp_socket_t;
  typedef boost::asio::ssl::stream<tcp_socket_t> ssl_socket_t;

public:
  template <typename Actor>
  explicit acceptor(Actor& a)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service())
    , accepting_(false)
    , scp_(this)
  {
  }
  
  ~acceptor()
  {
  }
  
public:
  void async_accept(socket& skt, message const& msg = message(as_accept))
  {
    start_accept(skt.get_impl(), msg);
  }

  void async_accept(boost::shared_ptr<tcp_socket_t> skt, message const& msg = message(as_accept))
  {
    tcp_skt_ = skt;
    start_accept(*tcp_skt_, msg);
  }

  void async_accept(boost::shared_ptr<ssl_socket_t> skt, message const& msg = message(as_accept))
  {
    ssl_skt_ = skt;
    start_accept(ssl_skt_->lowest_layer(), msg);
  }
  
  template <typename Socket>
  void async_accept(Socket& skt, message const& msg = message(as_accept))
  {
    start_accept(skt, msg);
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
  
private:
  static void handle_accept(guard_ptr guard, errcode_t const& ec)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }
    
    o->accepting_ = false;
    message& m = o->accept_msg_;
    m << ec;
    if (o->tcp_skt_.get() != 0)
    {
      m << o->tcp_skt_;
      o->tcp_skt_.reset();
    }
    else if (o->ssl_skt_.get() != 0)
    {
      m << o->ssl_skt_;
      o->ssl_skt_.reset();
    }
    o->pri_send2actor(m);
  }
  
  template <typename Socket>
  void start_accept(Socket& skt, message const& msg)
  {
    GCE_ASSERT(!accepting_);
    accept_msg_ = msg;
    impl_.async_accept(
      skt,
      snd_.wrap(
        boost::bind(
          &self_t::handle_accept, scp_.get(),
          boost::asio::placeholders::error
          )
        )
      );
    accepting_ = true;
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
  bool accepting_;

  message accept_msg_;
  boost::shared_ptr<tcp_socket_t> tcp_skt_;
  boost::shared_ptr<ssl_socket_t> ssl_skt_;
  message const msg_nil_;

  /// for quit
  base_t::scope<self_t> scp_;
};
} /// namespace tcp
} /// namespace asio
} /// namespace gce

#endif /// GCE_ASIO_ACCEPTOR_HPP
