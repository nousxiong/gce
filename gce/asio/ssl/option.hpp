///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_SSL_OPTION_HPP
#define GCE_ASIO_SSL_OPTION_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/ssl/ssl_option.adl.h>

namespace gce
{
namespace asio
{
typedef adl::ssl_option sslopt_t;
inline sslopt_t make_sslopt()
{
  return sslopt_t();
}
}
}

#endif /// GCE_ASIO_SSL_OPTION_HPP
