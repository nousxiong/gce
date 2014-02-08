///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/impl/tcp/acceptor.hpp>
#include <gce/actor/impl/tcp/socket.hpp>
#include <gce/actor/context.hpp>
#include <gce/detail/scope.hpp>
#include <gce/detail/cache_aligned_new.hpp>
#include <boost/bind.hpp>

namespace gce
{
namespace tcp
{
///----------------------------------------------------------------------------
acceptor::acceptor(
  strand_t* snd,
  std::string const& host,
  boost::uint16_t port
  )
  : snd_(snd)
  , acpr_(snd_->get_io_service())
  , host_(host)
  , port_(port)
{
}
///----------------------------------------------------------------------------
acceptor::~acceptor()
{
}
///----------------------------------------------------------------------------
void acceptor::bind()
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
///----------------------------------------------------------------------------
gce::detail::basic_socket* acceptor::accept(yield_t yield)
{
  socket* skt = GCE_CACHE_ALIGNED_NEW(socket)(snd_->get_io_service());
  detail::scope scp(boost::bind(&acceptor::delete_socket, this, skt));
  acpr_.async_accept(skt->get_socket(), yield);
  scp.reset();
  return skt;
}
///----------------------------------------------------------------------------
void acceptor::close()
{
  errcode_t ignore_ec;
  acpr_.close(ignore_ec);
}
///----------------------------------------------------------------------------
void acceptor::delete_socket(gce::detail::basic_socket* skt)
{
  GCE_CACHE_ALIGNED_DELETE(gce::detail::basic_socket, skt);
}
///----------------------------------------------------------------------------
}
}
