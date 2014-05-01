///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/impl/tcp/socket.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>

namespace gce
{
namespace tcp
{
///----------------------------------------------------------------------------
socket::socket(io_service_t& ios)
  : reso_(ios)
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
///----------------------------------------------------------------------------
socket::socket(io_service_t& ios, std::string const& host, std::string const& port)
  : reso_(ios)
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
///----------------------------------------------------------------------------
socket::~socket()
{
}
///----------------------------------------------------------------------------
void socket::init()
{
}
///----------------------------------------------------------------------------
void socket::send(
  byte_t const* header, std::size_t header_size,
  byte_t const* body, std::size_t body_size
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
///----------------------------------------------------------------------------
std::size_t socket::recv(byte_t* buf, std::size_t size, yield_t yield)
{
  return sock_.async_read_some(boost::asio::buffer(buf, size), yield);
}
///----------------------------------------------------------------------------
void socket::connect(yield_t yield)
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
///----------------------------------------------------------------------------
void socket::close()
{
  closed_ = true;
  if (!sending_)
  {
    close_socket();
  }
}
///----------------------------------------------------------------------------
void socket::wait_end(yield_t yield)
{
  waiting_end_ = true;
  if (sending_)
  {
    errcode_t ec;
    sync_.expires_from_now(infin);
    sync_.async_wait(yield[ec]);
  }
}
///----------------------------------------------------------------------------
void socket::reset()
{
  close_socket();
}
///----------------------------------------------------------------------------
void socket::close_socket()
{
  errcode_t ignore_ec;
  sock_.close(ignore_ec);
}
///----------------------------------------------------------------------------
void socket::begin_send()
{
  sending_ = true;
  std::swap(sending_buffer_, standby_buffer_);
  gce::detail::bytes_t const& bytes = send_buffer_[sending_buffer_];

  boost::asio::async_write(
    sock_,
    boost::asio::buffer(bytes.data(), bytes.size()),
    boost::bind(
      &socket::end_send, this,
      boost::asio::placeholders::error
      )
    );
}
///----------------------------------------------------------------------------
void socket::end_send(errcode_t const& errc)
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
///----------------------------------------------------------------------------
}
}
