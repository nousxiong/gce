///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_REMOTE_HPP
#define GCE_ACTOR_REMOTE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/socket.hpp>
#include <gce/actor/detail/acceptor.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/slice.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/detail/cache_pool.hpp>

namespace gce
{
namespace detail
{
inline aid_t connect(
  cache_pool* cac_pool, std::string const& ep,
  net_option opt, aid_t master
  )
{
  context& ctx = cac_pool->get_context();
  cache_pool* user = ctx.select_cache_pool();
  socket* s = cac_pool->get_socket();
  s->init(user, cac_pool, opt);
  aid_t ret = s->get_aid();
  s->connect(ep, master);
  return ret;
}

inline aid_t bind(
  cache_pool* cac_pool, std::string const& ep,
  net_option opt, aid_t master
  )
{
  context& ctx = cac_pool->get_context();
  cache_pool* user = ctx.select_cache_pool();
  acceptor* a = cac_pool->get_acceptor();
  a->init(user, cac_pool, opt);
  aid_t ret = a->get_aid();
  a->bind(ep, master);
  return ret;
}
}

/// connect
inline aid_t connect(mixin_t sire, std::string const& ep, net_option opt = net_option())
{
  aid_t ret = detail::connect(sire.select_cache_pool(), ep, opt, sire.get_aid());
  sire.add_link(detail::link_t(linked, ret));
  return ret;
}

inline aid_t connect(self_t sire, std::string const& ep, net_option opt = net_option())
{
  aid_t ret = detail::connect(sire.get_cache_pool(), ep, opt, sire.get_aid());
  sire.add_link(detail::link_t(linked, ret));
  return ret;
}

/// bind
inline void bind(mixin_t sire, std::string const& ep, net_option opt = net_option())
{
  aid_t aid = detail::bind(sire.select_cache_pool(), ep, opt, sire.get_aid());
  sire.add_link(detail::link_t(linked, aid));
}

inline void bind(self_t sire, std::string const& ep, net_option opt = net_option())
{
  aid_t aid = detail::bind(sire.get_cache_pool(), ep, opt, sire.get_aid());
  sire.add_link(detail::link_t(linked, aid));
}
}

#endif /// GCE_ACTOR_REMOTE_HPP
