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
  thread* thr,
  ctxid_t target,
  std::string const& ep,
  bool target_is_router,
  remote_func_list_t const& remote_func_list,
  net_option opt
  )
{
  if (target == ctxid_nil)
  {
    throw std::runtime_error("target invalid");
  }

  if (thr->get_ctxid() == ctxid_nil)
  {
    throw std::runtime_error(
      "ctxid haven't set, please set it before connect"
      );
  }

  socket* s = thr->get_cache_pool().get_socket();
  s->init(opt);
  s->connect(remote_func_list, target, ep, target_is_router);
}

inline void bind(
  thread* thr,
  std::string const& ep,
  bool is_router,
  remote_func_list_t const& remote_func_list,
  net_option opt
  )
{
  if (thr->get_ctxid() == ctxid_nil)
  {
    throw std::runtime_error(
      "ctxid haven't set, please set it before bind"
      );
  }

  acceptor* a = thr->get_cache_pool().get_acceptor();
  a->init(opt);
  a->bind(remote_func_list, ep, is_router);
}
}

/// connect
inline void connect(
  mixin_t sire,
  ctxid_t target, /// connect target
  std::string const& ep, /// endpoint
  bool target_is_router = false, /// if target is router, set it true
  net_option opt = net_option(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  context* ctx = sire.get_context();
  thread& thr = ctx->select_thread();
  thr.post(
    sire.get_pack(),
    boost::bind(
      &detail::connect, 
      &thr, target, ep, 
      target_is_router, remote_func_list, opt
      )
    );
}

inline void connect(
  self_t sire,
  ctxid_t target, /// connect target
  std::string const& ep, /// endpoint
  bool target_is_router = false, /// if target is router, set it true
  net_option opt = net_option(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  context* ctx = sire.get_context();
  thread& thr = ctx->select_thread();
  thr.post(
    sire.get_thread(),
    boost::bind(
      &detail::connect, 
      &thr, target, ep, 
      target_is_router, remote_func_list, opt
      )
    );
}

/// bind
inline void bind(
  mixin_t sire,
  std::string const& ep, /// endpoint
  bool is_router = false, /// if this bind is router, set it true
  remote_func_list_t const& remote_func_list = remote_func_list_t(),
  net_option opt = net_option()
  )
{
  context* ctx = sire.get_context();
  thread& thr = ctx->select_thread();
  thr.post(
    sire.get_pack(),
    boost::bind(
      &detail::bind, 
      &thr, ep, is_router, 
      remote_func_list, opt
      )
    );
}

inline void bind(
  self_t sire,
  std::string const& ep, /// endpoint
  bool is_router = false, /// if this bind is router, set it true
  remote_func_list_t const& remote_func_list = remote_func_list_t(),
  net_option opt = net_option()
  )
{
  context* ctx = sire.get_context();
  thread& thr = ctx->select_thread();
  thr.post(
    sire.get_thread(),
    boost::bind(
      &detail::bind, 
      &thr, ep, is_router, 
      remote_func_list, opt
      )
    );
}
}

#endif /// GCE_ACTOR_REMOTE_HPP
