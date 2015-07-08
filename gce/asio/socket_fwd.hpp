///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_SOCKET_FWD_HPP
#define GCE_ASIO_SOCKET_FWD_HPP

#include <gce/asio/config.hpp>

namespace gce
{
namespace asio
{
namespace tcp
{
static match_t const as_conn = atom("as_conn");
static match_t const as_recv = atom("as_recv");
static match_t const as_recv_some = atom("as_recv_some");
static match_t const as_recv_until = atom("as_recv_until");
static match_t const as_send = atom("as_send");
static match_t const as_send_some = atom("as_send_some");
}


namespace ssl
{
static match_t const as_handshake = atom("as_handshake");
static match_t const as_shutdown = atom("as_shutdown");
}
}
}

#endif /// GCE_ASIO_SOCKET_FWD_HPP
