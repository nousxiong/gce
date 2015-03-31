///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ASIO_HPP
#define GCE_ACTOR_ASIO_HPP

#include <gce/actor/config.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/spawn.hpp>

namespace gce
{
typedef boost::asio::io_service io_service_t;
typedef boost::asio::io_service::strand strand_t;
typedef boost::asio::system_timer timer_t;
typedef boost::asio::yield_context yield_t;

namespace detail
{
typedef boost::asio::coroutine coro_t;
}
}

#ifndef GCE_REENTER
# define GCE_REENTER(t) BOOST_ASIO_CORO_REENTER(t.coro())
#endif

#ifndef GCE_YIELD
# define GCE_YIELD BOOST_ASIO_CORO_YIELD
#endif

#endif /// GCE_ACTOR_ASIO_HPP
