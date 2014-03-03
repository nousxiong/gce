///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_IMPL_TCP_SOCKET_HPP
#define GCE_ACTOR_IMPL_TCP_SOCKET_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/basic_socket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/array.hpp>

namespace gce
{
namespace tcp
{
class socket
  : public gce::detail::basic_socket
{
public:
  explicit socket(io_service_t&);
  socket(io_service_t&, std::string const&, std::string const&);
  ~socket();

public:
  void init(gce::detail::cache_pool* user);
  void send(byte_t const*, std::size_t, byte_t const*, std::size_t);
  std::size_t recv(byte_t*, std::size_t, yield_t);
  void connect(yield_t);
  void close();
  void wait_end(yield_t);
  void reset();

  inline boost::asio::ip::tcp::socket& get_socket() { return sock_; }

private:
  void close_socket();
  void begin_send();
  void end_send(errcode_t const&);

private:
  gce::detail::cache_pool* user_;
  boost::asio::ip::tcp::resolver reso_;
  boost::asio::ip::tcp::socket sock_;
  std::string const host_;
  std::string const port_;

  bool closed_;
  bool reconn_;
  bool sending_;
  std::size_t sending_buffer_;
  std::size_t standby_buffer_;
  boost::array<gce::detail::bytes_t, 2> send_buffer_;

  timer_t sync_;
  bool waiting_end_;
};
typedef boost::shared_ptr<socket> socket_ptr;
}
}

#endif /// GCE_ACTOR_IMPL_TCP_SOCKET_HPP
