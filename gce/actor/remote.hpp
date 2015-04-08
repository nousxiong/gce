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
#include <gce/actor/actor.hpp>
#include <gce/actor/detail/remote.hpp>
#include <gce/actor/to_match.hpp>
#include <gce/detail/scope.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <utility>

namespace gce
{
typedef std::pair<match_t, detail::remote_func<context> > remote_func_t;
typedef std::vector<remote_func_t> remote_func_list_t;

template <typename Tag, typename Match, typename F>
inline remote_func_t make_remote_func(Match type, F f)
{
  return 
    std::make_pair(
      to_match(type),
      make_actor_function<Tag>(f)
      );
}
///------------------------------------------------------------------------------
/// Connect using given NONE coroutine_stackless_actor
///------------------------------------------------------------------------------
template <typename Ctxid>
inline errcode_t connect(
  threaded_actor sire,
  Ctxid target, /// connect target
  std::string const& ep, /// endpoint
  netopt_t opt = make_netopt(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  typedef context::socket_service_t service_t;
  typedef context::nonblocked_actor_t nonblocked_actor_t;
  context& ctx = sire.get_context();
  service_t& svc = ctx.select_service<service_t>(sire.get_service().get_index());
  detail::connect<context>(
    sire.get_aid(), svc, to_match(target), ep, opt, remote_func_list
    );

  detail::ctxid_pair_t ctxid_pr;
  errcode_t ec;
  aid_t skt = sire->recv(detail::msg_new_conn, ctxid_pr, ec);
  std::vector<nonblocked_actor_t*>& actor_list = sire.get_nonblocked_actor_list();
  BOOST_FOREACH(nonblocked_actor_t* s, actor_list)
  {
    s->get_service().register_socket(ctxid_pr, skt);
  }
  return ec;
}

template <typename Ctxid>
inline errcode_t connect(
  stackful_actor sire,
  Ctxid target, /// connect target
  std::string const& ep, /// endpoint
  netopt_t opt = make_netopt(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  typedef context::socket_service_t service_t;
  context& ctx = sire.get_context();
  service_t& svc = ctx.select_service<service_t>();

  detail::connect<context>(
    sire.get_aid(), svc, to_match(target), ep, opt, remote_func_list
    );
  detail::ctxid_pair_t ctxid_pr;
  errcode_t ec;
  aid_t skt = sire->recv(detail::msg_new_conn, ctxid_pr, ec);
  sire.get_service().register_socket(ctxid_pr, skt);
  return ec;
}

///------------------------------------------------------------------------------
/// Connect using given coroutine_stackless_actor
///------------------------------------------------------------------------------
template <typename Ctxid>
inline void connect(
  stackless_actor sire,
  Ctxid target, /// connect target
  std::string const& ep, /// endpoint
  errcode_t& ec,
  netopt_t opt = make_netopt(),
  remote_func_list_t const& remote_func_list = remote_func_list_t()
  )
{
  typedef context::socket_service_t service_t;
  context& ctx = sire.get_context();
  service_t& svc = ctx.select_service<service_t>();
  detail::connect<context>(
    sire.get_aid(), svc, to_match(target), ep, opt, remote_func_list
    );

  pattern patt;
  patt.match_list_.push_back(detail::msg_new_conn);
  sire.recv(
    boost::bind(
      &detail::handle_connect<context>, _arg1, _arg2, _arg3, 
      boost::ref(sire.get_service()), boost::ref(ec)
      ), 
    patt
    );
}

///------------------------------------------------------------------------------
/// Bind using given NONE coroutine_stackless_actor
///------------------------------------------------------------------------------
template <typename ActorRef>
inline void bind(
  ActorRef sire,
  std::string const& ep, /// endpoint
  remote_func_list_t const& remote_func_list = remote_func_list_t(),
  netopt_t opt = make_netopt()
  )
{
  typedef context::acceptor_service_t service_t;
  context& ctx = sire.get_context();
  service_t& svc = ctx.select_service<service_t>();
  detail::bind<context>(sire.get_aid(), svc, ep, opt, remote_func_list);
  sire->recv(detail::msg_new_bind);
}

///------------------------------------------------------------------------------
/// Bind using given coroutine_stackless_actor
///------------------------------------------------------------------------------
inline void bind(
  stackless_actor sire,
  std::string const& ep, /// endpoint
  remote_func_list_t const& remote_func_list = remote_func_list_t(),
  netopt_t opt = make_netopt()
  )
{
  typedef context::acceptor_service_t service_t;
  context& ctx = sire.get_context();
  service_t& svc = ctx.select_service<service_t>();
  detail::bind<context>(sire.get_aid(), svc, ep, opt, remote_func_list);

  pattern patt;
  patt.match_list_.push_back(detail::msg_new_bind);
  sire.recv(
    boost::bind(
      &detail::handle_bind<context>, _arg1, _arg2, _arg3
      ), 
    patt
    );
}
}

#endif /// GCE_ACTOR_REMOTE_HPP
