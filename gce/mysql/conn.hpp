///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_CONN_HPP
#define GCE_MYSQL_CONN_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/conn.adl.h>

namespace gce
{
namespace mysql
{
namespace detail
{
struct conn_impl;
}
typedef adl::conn conn_t;
static conn_t make_conn()
{
  return conn_t();
}

static detail::conn_impl* get_conn_impl(conn_t const& c)
{
  return (detail::conn_impl*)c.ptr_;
}
}
}

#endif /// GCE_MYSQL_CONN_HPP
