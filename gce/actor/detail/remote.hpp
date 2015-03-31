///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_REMOTE_HPP
#define GCE_ACTOR_DETAIL_REMOTE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/socket_actor.hpp>
#include <gce/actor/detail/acceptor_actor.hpp>

namespace gce
{
namespace detail
{
template <typename Context>
inline void connect_impl(
  aid_t sire,
  typename Context::socket_service_t& svc,
  ctxid_t target,
  std::string const& ep,
  std::vector<std::pair<match_t, remote_func<Context> > > const& remote_func_list,
  netopt_t opt
  )
{
  typedef Context context_t;
  typedef typename context_t::socket_actor_t socket_actor_t;

  GCE_VERIFY(target != ctxid_nil);
  GCE_VERIFY(svc.get_context().get_ctxid() != ctxid_nil)
    .msg("ctxid haven't set, please set it before connect");

  socket_actor_t* s = svc.make_actor();
  s->init(opt);
  s->connect(sire, remote_func_list, target, ep, opt.is_router != 0);
}

template <typename Context>
inline void bind_impl(
  aid_t sire,
  typename Context::acceptor_service_t& svc,
  std::string const& ep,
  std::vector<std::pair<match_t, remote_func<Context> > > const& remote_func_list,
  netopt_t opt
  )
{
  typedef Context context_t;
  typedef typename context_t::acceptor_actor_t acceptor_actor_t;

  GCE_VERIFY(svc.get_context().get_ctxid() != ctxid_nil)
    .msg("ctxid haven't set, please set it before bind");

  acceptor_actor_t* a = svc.make_actor();
  a->init(opt);
  a->bind(sire, remote_func_list, ep);
}

/// connect
template <typename Context>
inline void connect(
  aid_t sire,
  typename Context::socket_service_t& svc,
  ctxid_t target, /// connect target
  std::string const& ep, /// endpoint
  netopt_t opt,
  std::vector<std::pair<match_t, remote_func<Context> > > const& remote_func_list =
    std::vector<std::pair<match_t, remote_func<Context> > >()
  )
{
  typedef Context context_t;
  svc.get_strand().post(
    boost::bind(
      &connect_impl<context_t>,
      sire, boost::ref(svc), target, 
      ep, remote_func_list, opt
      )
    );
}

template <typename Context>
inline void handle_connect(
  actor_ref<stackless, Context> self, aid_t skt, 
  message msg, typename Context::stackless_service_t& sire, errcode_t& ec
  )
{
  if (skt)
  {
    ctxid_pair_t ctxid_pr;
    msg >> ctxid_pr >> ec;
    sire.register_socket(ctxid_pr, skt);
  }

  self.resume();
}

template <typename Context>
inline void handle_bind(actor_ref<stackless, Context> self, aid_t acpr, message)
{
  self.resume();
}

// bind
template <typename Context>
inline void bind(
  aid_t sire,
  typename Context::acceptor_service_t& svc,
  std::string const& ep, /// endpoint
  netopt_t opt,
  std::vector<std::pair<match_t, remote_func<Context> > > const& remote_func_list =
    std::vector<std::pair<match_t, remote_func<Context> > >()
  )
{
  typedef Context context_t;
  svc.get_strand().post(
    boost::bind(
      &bind_impl<context_t>,
      sire, boost::ref(svc), ep,
      remote_func_list, opt
      )
    );
}
}
}

#endif /// GCE_ACTOR_DETAIL_REMOTE_HPP
