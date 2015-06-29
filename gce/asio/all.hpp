///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_ALL_HPP
#define GCE_ASIO_ALL_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/asio.hpp>
#include <gce/asio/signal.hpp>
#include <gce/asio/timer.hpp>
#include <gce/asio/serial_port.hpp>
#include <gce/asio/tcp/resolver.hpp>
#include <gce/asio/tcp/acceptor.hpp>
#include <gce/asio/tcp/socket.hpp>
#ifdef GCE_LUA
# include <gce/asio/cpp2lua.hpp>
#endif
#ifdef GCE_OPENSSL
# include <gce/asio/ssl/stream.hpp>
#endif
#include <gce/asio/session.hpp>
#include <gce/asio/parser/simple.hpp>

#endif /// GCE_ASIO_ALL_HPP
