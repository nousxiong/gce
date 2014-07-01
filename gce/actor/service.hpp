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
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/service_id.hpp>

namespace gce
{
namespace detail
{
template <typename Actor>
inline void register_service(Actor& a, match_t name)
{
  cache_pool* user = a.get_cache_pool();
  context& ctx = user->get_context();
  user->register_service(name, a.get_aid());
  ctx.register_service(name, a.get_aid(), user->get_index());
}

template <typename Actor>
inline void deregister_service(Actor& a, match_t name)
{
  cache_pool* user = a.get_cache_pool();
  context& ctx = user->get_context();
  user->deregister_service(name, a.get_aid());
  ctx.deregister_service(name, a.get_aid(), user->get_index());
}
}

inline void register_service(actor<stackful>& self, match_t name)
{
  detail::register_service(self, name);
}

inline void deregister_service(actor<stackful>& self, match_t name)
{
  detail::deregister_service(self, name);
}

inline void register_service(actor<stackless>& self, match_t name)
{
  detail::register_service(self, name);
}

inline void deregister_service(actor<stackless>& self, match_t name)
{
  detail::deregister_service(self, name);
}

inline gce::svcid_t make_svcid(ctxid_t ctxid, match_t name)
{
  return svcid_t(ctxid, name);
}
}

#endif /// GCE_ACTOR_SERVICE_HPP
