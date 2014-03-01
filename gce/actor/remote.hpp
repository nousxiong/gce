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
#include <gce/actor/net_option.hpp>
#include <gce/actor/detail/cache_pool.hpp>

namespace gce
{
namespace detail
{
template <typename Sire>
inline aid_t connect(
  Sire& sire, cache_pool* user, cache_pool* owner,
  std::string const& ep, net_option opt
  )
{
  context& ctx = owner->get_context();
  if (!user)
  {
    user = ctx.select_cache_pool();
  }

  socket* s = owner->get_socket();
  s->init(user, owner, opt);
  aid_t aid = s->get_aid();
  sire.add_link(detail::link_t(linked, aid));
  s->connect(ep, sire.get_aid());
  return aid;
}

template <typename Sire>
inline void bind(
  Sire& sire, cache_pool* user, cache_pool* owner,
  std::string const& ep, net_option opt
  )
{
  context& ctx = owner->get_context();
  if (!user)
  {
    user = ctx.select_cache_pool();
  }

  acceptor* a = owner->get_acceptor();
  a->init(user, owner, opt);
  aid_t aid = a->get_aid();
  sire.add_link(detail::link_t(linked, aid));
  a->bind(ep, sire.get_aid());
}
}

/// connect
inline aid_t connect(mixin_t sire, std::string const& ep, net_option opt = net_option())
{
  detail::cache_pool* owner = sire.select_cache_pool();
  context& ctx = owner->get_context();

  ///   In boost 1.54 & 1.55, boost::asio::spawn will crash
  /// When spwan multi-coros at main thread
  /// With multi-threads run io_service::run (vc11)
  ///   I'don't know this wheather or not a bug.
  ///   So, if using mixin(means in main or other user thread(s)),
  /// We spawn actor(s) with only one cache_pool(means only one asio::strand).
  detail::cache_pool* user = ctx.select_cache_pool(0);
  return detail::connect(sire, user, owner, ep, opt);
}

inline aid_t connect(self_t sire, std::string const& ep, net_option opt = net_option())
{
  detail::cache_pool* user = 0;
  detail::cache_pool* owner = sire.get_cache_pool();
  return detail::connect(sire, user, owner, ep, opt);
}

/// bind
inline void bind(mixin_t sire, std::string const& ep, net_option opt = net_option())
{
  detail::cache_pool* owner = sire.select_cache_pool();
  context& ctx = owner->get_context();

  ///   In boost 1.54 & 1.55, boost::asio::spawn will crash
  /// When spwan multi-coros at main thread
  /// With multi-threads run io_service::run (vc11)
  ///   I'don't know this wheather or not a bug.
  ///   So, if using mixin(means in main or other user thread(s)),
  /// We spawn actor(s) with only one cache_pool(means only one asio::strand).
  detail::cache_pool* user = ctx.select_cache_pool(0);
  detail::bind(sire, user, owner, ep, opt);
}

inline void bind(self_t sire, std::string const& ep, net_option opt = net_option())
{
  detail::cache_pool* user = 0;
  detail::cache_pool* owner = sire.get_cache_pool();
  detail::bind(sire, user, owner, ep, opt);
}
}

#endif /// GCE_ACTOR_REMOTE_HPP
