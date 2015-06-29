///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_DETAIL_SESSION_FWD_HPP
#define GCE_ASIO_DETAIL_SESSION_FWD_HPP

#include <gce/asio/config.hpp>

namespace gce
{
namespace asio
{
static match_t const sn_open = atom("sn_open");
static match_t const sn_recv = atom("sn_recv");
static match_t const sn_idle = atom("sn_idle");
static match_t const sn_close = atom("sn_close");

namespace detail
{
static match_t const api_open = atom("api_open");
static match_t const api_close = atom("api_close");
}
}
}

#endif /// GCE_ASIO_DETAIL_SESSION_FWD_HPP
