///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_IMPL_TCP_ACCEPTOR_HPP
#define GCE_ACTOR_IMPL_TCP_ACCEPTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/basic_acceptor.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace gce
{
namespace detail
{
class basic_socket;
}
namespace tcp
{
class acceptor
  : public gce::detail::basic_acceptor
{
public:
  acceptor(strand_t&, std::string const&, boost::uint16_t);
  ~acceptor();

public:
  void bind();
  gce::detail::basic_socket* accept(yield_t);
  void close();

private:
  void delete_socket(gce::detail::basic_socket*);

private:
  strand_t& snd_;
  boost::asio::ip::tcp::acceptor acpr_;
  std::string const host_;
  boost::uint16_t const port_;
};
}
}

#endif /// GCE_ACTOR_IMPL_TCP_ACCEPTOR_HPP
