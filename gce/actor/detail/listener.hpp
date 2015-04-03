///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_LISTENER_HPP
#define GCE_ACTOR_DETAIL_LISTENER_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_fwd.hpp>

namespace gce
{
namespace detail
{
struct pack;
class listener
{
public:
  virtual void on_recv(pack&) = 0;
  virtual void on_addon_recv(pack&) = 0;
};
}
}

#endif /// GCE_ACTOR_DETAIL_LISTENER_HPP
