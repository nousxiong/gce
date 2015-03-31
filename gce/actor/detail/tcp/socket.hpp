///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_TCP_SOCKET_HPP
#define GCE_ACTOR_DETAIL_TCP_SOCKET_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/asio.hpp>
#include <gce/actor/detail/basic_socket.hpp>
#include <gce/detail/bytes.hpp>
#include <gce/integer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <string>

namespace gce
{
typedef std::basic_string<byte_t, std::char_traits<byte_t>, std::allocator<byte_t> > bytes_t;
namespace detail
{
namespace tcp
{
class socket
  : public basic_socket
{
public:
  explicit socket(io_service_t& ios)
    : snd_(0)
    , reso_(ios)
    , sock_(ios)
    , closed_(false)
    , reconn_(false)
    , sending_(false)
    , sending_buffer_(0)
    , standby_buffer_(1)
    , sync_(ios)
    , waiting_end_(false)
  {
  }

  socket(io_service_t& ios, std::string const& host, std::string const& port)
    : snd_(0)
    , reso_(ios)
    , sock_(ios)
    , host_(host)
    , port_(port)
    , closed_(false)
    , reconn_(false)
    , sending_(false)
    , sending_buffer_(0)
    , standby_buffer_(1)
    , sync_(ios)
    , waiting_end_(false)
  {
  }

  ~socket()
  {
  }

public:
  void init(strand_t& snd)
  {
    snd_ = &snd;
  }

  void send(
    byte_t const* header, size_t header_size,
    byte_t const* body, size_t body_size
    )
  {
    if (!waiting_end_)
    {
      gce::detail::bytes_t& send_buf = send_buffer_[standby_buffer_];
      send_buf.append(header, header_size);
      send_buf.append(body, body_size);

      if (!sending_)
      {
        begin_send();
      }
    }
  }

  size_t recv(byte_t* buf, size_t size, yield_t yield)
  {
    return sock_.async_read_some(boost::asio::buffer(buf, size), yield);
  }

  void connect(yield_t yield)
  {
    if (sending_)
    {
      reconn_ = true;
    }
    close_socket();
    boost::asio::ip::tcp::resolver::query query(host_, port_);
    boost::asio::ip::tcp::resolver::iterator itr = reso_.async_resolve(query, yield);
    if (yield.ec_ && *yield.ec_)
    {
      return;
    }
    boost::asio::async_connect(sock_, itr, yield);
  }

  void close()
  {
    closed_ = true;
    if (!sending_)
    {
      close_socket();
    }
  }

  void wait_end(yield_t yield)
  {
    waiting_end_ = true;
    if (sending_)
    {
      errcode_t ec;
      sync_.expires_from_now(to_chrono(infin));
      sync_.async_wait(yield[ec]);
    }
  }

  void reset()
  {
    close_socket();
  }

  boost::asio::ip::tcp::socket& get_socket()
  { 
    return sock_;
  }

private:
  void close_socket()
  {
    errcode_t ignore_ec;
    sock_.close(ignore_ec);
  }

  void begin_send()
  {
    sending_ = true;
    strand_t& snd = *snd_;
    std::swap(sending_buffer_, standby_buffer_);
    gce::detail::bytes_t const& bytes = send_buffer_[sending_buffer_];

    boost::asio::async_write(
      sock_,
      boost::asio::buffer(bytes.data(), bytes.size()),
      snd.wrap(
        boost::bind(
          &socket::end_send, this,
          boost::asio::placeholders::error
          )
        )
      );
  }

  void end_send(errcode_t const& errc)
  {
    sending_ = false;
    send_buffer_[sending_buffer_].clear();

    if (!errc && !send_buffer_[standby_buffer_].empty())
    {
      begin_send();
    }
    else if (closed_)
    {
      close();
    }

    if (closed_ && !sending_)
    {
      errcode_t ignore_ec;
      sync_.cancel(ignore_ec);
    }

    reconn_ = false;
  }

private:
  strand_t* snd_;
  boost::asio::ip::tcp::resolver reso_;
  boost::asio::ip::tcp::socket sock_;
  std::string const host_;
  std::string const port_;

  bool closed_;
  bool reconn_;
  bool sending_;
  size_t sending_buffer_;
  size_t standby_buffer_;
  boost::array<gce::detail::bytes_t, 2> send_buffer_;

  timer_t sync_;
  bool waiting_end_;
};
typedef boost::shared_ptr<socket> socket_ptr;
} /// namespace tcp
} /// namespace detail
} /// namespace gce

#endif /// GCE_ACTOR_DETAIL_TCP_SOCKET_HPP
