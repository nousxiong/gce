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

namespace gce
{
namespace detail
{
class cache_pool;
class basic_socket
{
public:
  basic_socket() {}
  virtual ~basic_socket() {}

public:
  virtual void init(cache_pool*) = 0;
  virtual void send(byte_t const*, std::size_t, byte_t const*, std::size_t) = 0;
  virtual std::size_t recv(byte_t*, std::size_t, yield_t) = 0;
  virtual void connect(yield_t) = 0;
  virtual void close() = 0;
  virtual void wait_end(yield_t) = 0;
  virtual void reset() = 0;
};
}
}

#endif /// GCE_ACTOR_DETAIL_BASIC_SOCKET_HPP
