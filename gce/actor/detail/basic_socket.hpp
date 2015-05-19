///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_BASIC_SOCKET_HPP
#define GCE_ACTOR_DETAIL_BASIC_SOCKET_HPP

#include <gce/actor/config.hpp>
#include <boost/shared_ptr.hpp>

namespace gce
{
class message;
namespace detail
{
class basic_socket
{
public:
  basic_socket() {}
  virtual ~basic_socket() {}

public:
  virtual void init(strand_t&) = 0;
  virtual void send(message const&) = 0;
  virtual size_t recv(byte_t*, size_t, yield_t) = 0;
  virtual void connect(yield_t) = 0;
  virtual void close() = 0;
  virtual void wait_end(yield_t) = 0;
  virtual void reset() = 0;
};
typedef boost::shared_ptr<basic_socket> socket_ptr;
}
}

#endif /// GCE_ACTOR_DETAIL_BASIC_SOCKET_HPP
