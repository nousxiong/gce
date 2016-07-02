///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_BASIC_ACCEPTOR_HPP
#define GCE_ACTOR_DETAIL_BASIC_ACCEPTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/basic_socket.hpp>

namespace gce
{
namespace detail
{
class basic_acceptor
{
public:
  basic_acceptor() {}
  virtual ~basic_acceptor() {}

public:
  virtual int32_t bind() = 0; /// Return port, if <= 0 error.
  virtual socket_ptr accept(yielder) = 0;
  virtual void close() = 0;
};
}
}

#endif /// GCE_ACTOR_DETAIL_BASIC_ACCEPTOR_HPP

