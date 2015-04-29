///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_TIMER_HPP
#define GCE_ASIO_TIMER_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/detail/basic_timer.hpp>
#include <boost/asio/system_timer.hpp>

namespace gce
{
namespace asio
{
typedef detail::basic_timer<boost::asio::system_timer, 0> system_timer;
} /// namespace asio
} /// namespace gce

#endif /// GCE_ASIO_TIMER_HPP
