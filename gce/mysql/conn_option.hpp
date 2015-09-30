///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_CONN_OPTION_HPP
#define GCE_MYSQL_CONN_OPTION_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/conn_option.adl.h>

namespace gce
{
namespace mysql
{
typedef adl::conn_option connopt_t;
static connopt_t make_connopt()
{
  return connopt_t();
}
}
}

#endif /// GCE_MYSQL_CONN_OPTION_HPP
