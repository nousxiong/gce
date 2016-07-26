///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_NEWDECIMAL_HPP
#define GCE_MYSQL_NEWDECIMAL_HPP

#include <gce/mysql/config.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace gce
{
namespace mysql
{
/// just a type for fetch
struct newdecimal
{
  double val_;
};
}

inline std::string to_string(mysql::newdecimal const& o)
{
  std::string str;
  str += "nd<";
  str += boost::lexical_cast<intbuf_t>(o.val_).cbegin();
  str += ">";
  return str;
}

template <>
struct tostring<mysql::newdecimal>
{
  static std::string convert(mysql::newdecimal const& o)
  {
    return to_string(o);
  }
};
}

#endif /// GCE_MYSQL_NEWDECIMAL_HPP
