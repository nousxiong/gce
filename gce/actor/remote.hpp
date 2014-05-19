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
#include <gce/actor/coroutine_stackfull_actor.hpp>
#include <gce/actor/thread_mapped_actor.hpp>
#include <gce/actor/slice.hpp>
#include <gce/actor/send.hpp>
#include <gce/actor/recv.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/detail/scope.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <utility>

namespace gce
{
namespace detail
{
inline void connect_impl(
  aid_t sire,
  cache_pool* user,
  ctxid_t target,
  std::string const& ep,
  bool target_is_router,
  remote_func_list_t const& remote_func_list,
  net_option opt
  )
{
  BOOST_ASSERT(target != ctxid_nil);
  BOOST_ASSERT_MSG(
    user->get_context().get_attributes().id_ != ctxid_nil, 
    "ctxid haven't set, please set it before connect"
    );

  socket* s = user->get_socket();
  s->init(opt);
  s->connect(
    sire, remote_func_list, target,
    ep, target_is_router
    );
}

inline void bind_impl(
  aid_t sire,
  cache_pool* user,
  std::string const& ep,
  bool is_router,
  remote_func_list_t const& remote_func_list,
  net_option opt
  )
{
  if (user->get_context().get_attributes().id_ == ctxid_nil)
  {
    throw std::runtime_error(
      "ctxid haven't set, please set it before bind"
      );
  }

  acceptor* a = user->get_acceptor();
  a->init(opt);
  a->bind(sire, remote_func_list, ep, is_router);
}

/// connect
template <typename Sire>
inline void connect(
  Sire& sire,
  cache_pool* user,
  ctxid_t target, /// connect target
  std::string const& ep, /// endpoint
  bool target_is_router = false, /// if target is router, set it true
  net_option opt = net_option(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  user->get_strand().post(
    boost::bind(
      &connect_impl,
      sire.get_aid(), user, target, ep,
      target_is_router, remote_func_list, opt
      )
    );
}
}

/// connect
template <typename Sire>
inline void connect(
  Sire& sire,
  ctxid_t target, /// connect target
  std::string const& ep, /// endpoint
  bool target_is_router = false, /// if target is router, set it true
  net_option opt = net_option(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  detail::cache_pool* user = sire.get_cache_pool();
  detail::connect(sire, user, target, ep, target_is_router, opt, remote_func_list);
  ctxid_pair_t ctxid_pr;
  aid_t skt = recv(sire, detail::msg_new_conn, ctxid_pr);
  std::vector<slice*>& slice_list = sire.get_slice_list();
  BOOST_FOREACH(slice* s, slice_list)
  {
    s->get_cache_pool()->register_socket(ctxid_pr, skt);
  }
}

/// connect
inline void connect(
  self_t sire,
  ctxid_t target, /// connect target
  std::string const& ep, /// endpoint
  bool target_is_router = false, /// if target is router, set it true
  net_option opt = net_option(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  detail::cache_pool* user = sire.get_context()->select_cache_pool();
  detail::connect(sire, user, target, ep, target_is_router, opt, remote_func_list);
  ctxid_pair_t ctxid_pr;
  aid_t skt = recv(sire, detail::msg_new_conn, ctxid_pr);
  sire.get_cache_pool()->register_socket(ctxid_pr, skt);
}

/// bind
template <typename Sire>
inline void bind(
  Sire& sire,
  std::string const& ep, /// endpoint
  bool is_router = false, /// if this bind is router, set it true
  remote_func_list_t const& remote_func_list = remote_func_list_t(),
  net_option opt = net_option()
  )
{
  detail::cache_pool* user = sire.get_context()->select_cache_pool();
  user->get_strand().post(
    boost::bind(
      &detail::bind_impl,
      sire.get_aid(), user, ep, is_router,
      remote_func_list, opt
      )
    );
  recv(sire, detail::msg_new_bind);
}
}

#endif /// GCE_ACTOR_REMOTE_HPP
