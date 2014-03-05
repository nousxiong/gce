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
#include <vector>
#include <utility>

namespace gce
{
namespace detail
{
inline void connect(
  cache_pool* user, cache_pool* owner,
  ctxid_t target, std::string const& ep, net_option opt
  )
{
  context& ctx = owner->get_context();
  if (!user)
  {
    user = ctx.select_cache_pool();
  }

  if (target == ctxid_nil)
  {
    throw std::runtime_error("target invalid");
  }

  if (user->get_ctxid() == ctxid_nil)
  {
    throw std::runtime_error(
      "ctxid haven't set, please set it before connect"
      );
  }

  socket* s = owner->get_socket();
  s->init(user, owner, opt);
  s->connect(target, ep);
}

inline void bind(
  cache_pool* user,
  cache_pool* owner, std::string const& ep,
  remote_func_list_t const& remote_func_list,
  net_option opt
  )
{
  if (remote_func_list.empty())
  {
    throw std::runtime_error("remote_func_list must not empty");
  }

  context& ctx = owner->get_context();
  if (!user)
  {
    user = ctx.select_cache_pool();
  }

  if (user->get_ctxid() == ctxid_nil)
  {
    throw std::runtime_error(
      "ctxid haven't set, please set it before bind"
      );
  }

  acceptor* a = owner->get_acceptor();
  a->init(user, owner, opt);
  a->bind(remote_func_list, ep);
}
}

/// connect
inline void connect(
  mixin_t sire, ctxid_t target,
  std::string const& ep, net_option opt = net_option()
  )
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
  detail::connect(user, owner, target, ep, opt);
}

inline void connect(
  self_t sire, ctxid_t target,
  std::string const& ep, net_option opt = net_option()
  )
{
  detail::cache_pool* user = 0;
  detail::cache_pool* owner = sire.get_cache_pool();
  detail::connect(user, owner, target, ep, opt);
}

/// bind
inline void bind(
  mixin_t sire, std::string const& ep,
  remote_func_list_t const& remote_func_list,
  net_option opt = net_option()
  )
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
  detail::bind(user, owner, ep, remote_func_list, opt);
}

inline void bind(
  self_t sire, std::string const& ep,
  remote_func_list_t const& remote_func_list,
  net_option opt = net_option()
  )
{
  detail::cache_pool* user = 0;
  detail::cache_pool* owner = sire.get_cache_pool();
  detail::bind(user, owner, ep, remote_func_list, opt);
}
}

#endif /// GCE_ACTOR_REMOTE_HPP
