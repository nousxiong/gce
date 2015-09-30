///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_EXCEPTION_HPP
#define GCE_MYSQL_EXCEPTION_HPP

#include <gce/mysql/config.hpp>

namespace gce
{
namespace mysql
{
class init_exception
  : public exception
{
public:
  explicit init_exception(char const* msg)
    : exception(msg)
  {
  }
};
}
}

#endif /// GCE_MYSQL_EXCEPTION_HPP
