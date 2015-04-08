///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ERROR_CODE_HPP
#define GCE_ACTOR_ERROR_CODE_HPP

#include <gce/actor/config.hpp>
#include <boost/system/error_code.hpp>

namespace gce
{
typedef boost::system::error_code errcode_t;

inline std::string to_string(errcode_t const& ec)
{
  std::string str;
  str += "ec<";
  str += boost::lexical_cast<intbuf_t>(ec.value()).cbegin();
  str += ".";
  str += ec.message();
  str += ">";
  return str;
}

template <>
struct tostring<errcode_t>
{
  static std::string convert(errcode_t const& o)
  {
    return to_string(o);
  }
};
}

inline std::ostream& operator<<(std::ostream& strm, gce::errcode_t const& ec)
{
  strm << gce::to_string(ec);
  return strm;
}

#endif /// GCE_ACTOR_ERROR_CODE_HPP
