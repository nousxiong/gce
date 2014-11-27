///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SERVICE_HPP
#define GCE_ACTOR_DETAIL_SERVICE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>

namespace gce
{
namespace detail
{
template <typename Service>
inline void register_service(aid_t aid, Service& svc, match_t name)
{
  typedef typename Service::context_t context_t;
  context_t& ctx = svc.get_context();
  svc.register_service(name, aid);
  ctx.register_service(name, aid, svc.get_type(), svc.get_index());
}

template <typename Service>
inline void deregister_service(aid_t aid, Service& svc, match_t name)
{
  typedef typename Service::context_t context_t;
  context_t& ctx = svc.get_context();
  svc.deregister_service(name, aid);
  ctx.deregister_service(name, aid, svc.get_type(), svc.get_index());
}
}
}

#endif /// GCE_ACTOR_DETAIL_SERVICE_HPP
