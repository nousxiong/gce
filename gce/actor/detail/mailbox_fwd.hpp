///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_MAILBOX_FWD_HPP
#define GCE_ACTOR_DETAIL_MAILBOX_FWD_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/detail/exit.hpp>
#include <boost/variant/variant.hpp>

namespace gce
{
namespace detail
{
typedef boost::variant<aid_t, request_t, exit_t> recv_t;
}
}

#endif /// GCE_ACTOR_DETAIL_MAILBOX_FWD_HPP
