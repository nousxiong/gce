///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_SN_OPTION_HPP
#define GCE_ASIO_SN_OPTION_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/sn_option.adl.h>

namespace gce
{
namespace asio
{
typedef adl::sn_option snopt_t;
inline snopt_t make_snopt()
{
  snopt_t opt;
  opt.idle_period = seconds(30);
  opt.bigmsg_size = GCE_SOCKET_BIG_MSG_SIZE;
  return opt;
}
}
}

#endif /// GCE_ASIO_SN_OPTION_HPP
