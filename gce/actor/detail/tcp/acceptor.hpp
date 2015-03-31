///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_TCP_ACCEPTOR_HPP
#define GCE_ACTOR_DETAIL_TCP_ACCEPTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/basic_acceptor.hpp>
#include <gce/actor/detail/tcp/socket.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace gce
{
namespace detail
{
namespace tcp
{
class acceptor
  : public gce::detail::basic_acceptor
{
public:
  acceptor(strand_t& snd, std::string const& host, uint16_t port)
    : snd_(snd)
    , acpr_(snd_.get_io_service())
    , host_(host)
    , port_(port)
  {
  }

  ~acceptor()
  {
  }

public:
  void bind()
  {
    boost::asio::ip::address addr;
    addr.from_string(host_);
    boost::asio::ip::tcp::endpoint ep(addr, port_);
    acpr_.open(ep.protocol());

    acpr_.set_option(boost::asio::socket_base::reuse_address(true));
    acpr_.bind(ep);

    acpr_.set_option(boost::asio::socket_base::receive_buffer_size(640000));
    acpr_.set_option(boost::asio::socket_base::send_buffer_size(640000));

    acpr_.listen(1024);

    acpr_.set_option(boost::asio::ip::tcp::no_delay(true));
    acpr_.set_option(boost::asio::socket_base::keep_alive(true));
    acpr_.set_option(boost::asio::socket_base::enable_connection_aborted(true));
  }

  gce::detail::socket_ptr accept(yield_t yield)
  {
    socket_ptr skt(new socket(snd_.get_io_service()));
    acpr_.async_accept(skt->get_socket(), yield);
    return skt;
  }

  void close()
  {
    errcode_t ignore_ec;
    acpr_.close(ignore_ec);
  }

private:
  strand_t& snd_;
  boost::asio::ip::tcp::acceptor acpr_;
  std::string const host_;
  uint16_t const port_;
};
}
}
}

#endif /// GCE_ACTOR_DETAIL_TCP_ACCEPTOR_HPP
