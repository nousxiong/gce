///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_SERVICE_HPP
#define GCE_ACTOR_SERVICE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/service_id.hpp>
#include <gce/actor/detail/service.hpp>
#include <gce/actor/to_match.hpp>

namespace gce
{
template <typename Match>
inline void register_service(stackful_actor self, Match name)
{
  detail::register_service(
    self.get_aid(), self.get_service(), to_match(name)
    );
}

template <typename Match>
inline void deregister_service(stackful_actor self, Match name)
{
  detail::deregister_service(
    self.get_aid(), self.get_service(), to_match(name)
    );
}

template <typename Match>
inline void register_service(stackless_actor self, Match name)
{
  detail::register_service(
    self.get_aid(), self.get_service(), to_match(name)
    );
}

template <typename Match>
inline void deregister_service(stackless_actor self, Match name)
{
  detail::deregister_service(
    self.get_aid(), self.get_service(), to_match(name)
    );
}
}

#endif /// GCE_ACTOR_SERVICE_HPP
