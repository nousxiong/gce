///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_WRAP_HPP
#define GCE_ACTOR_DETAIL_WRAP_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/recv.hpp>
#include <gce/actor/detail/sender.hpp>

namespace gce
{
namespace detail
{
template <typename ActorRef, bool is_stackless>
struct actor_wrap
{
};

template <typename ActorRef>
struct actor_wrap<ActorRef, false>
  : public sender<ActorRef>
{
  typedef sender<ActorRef> base_t;
  typedef ActorRef actor_ref_t;

  aid_t recv(duration_t tmo = infin)
  {
    message msg;
    pattern patt(tmo);
    return recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
  }

  template <typename Match>
  aid_t recv(Match type, duration_t tmo = infin)
  {
    message msg;
    pattern patt(tmo);
    make_pattern(patt, type);
    return recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
  }

  template <typename Match, typename A1>
  aid_t recv(Match type, A1& a1, duration_t tmo = infin)
  {
    message msg;
    pattern patt(tmo);
    make_pattern(patt, type);
    aid_t sender = recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
    if (sender)
    {
      msg >> a1;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2>
  aid_t recv(Match type, A1& a1, A2& a2, duration_t tmo = infin)
  {
    message msg;
    pattern patt(tmo);
    make_pattern(patt, type);
    aid_t sender = recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
    if (sender)
    {
      msg >> a1 >> a2;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2, typename A3>
  aid_t recv(Match type, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    message msg;
    pattern patt(tmo);
    make_pattern(patt, type);
    aid_t sender = recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
    if (sender)
    {
      msg >> a1 >> a2 >> a3;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  aid_t recv(Match type, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    message msg;
    pattern patt(tmo);
    make_pattern(patt, type);
    aid_t sender = recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
    if (sender)
    {
      msg >> a1 >> a2 >> a3 >> a4;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t recv(Match type, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    message msg;
    pattern patt(tmo);
    make_pattern(patt, type);
    aid_t sender = recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
    if (sender)
    {
      msg >> a1 >> a2 >> a3 >> a4 >> a5;
    }
    return sender;
  }

  aid_t respond(resp_t res, duration_t tmo = infin)
  {
    message msg;
    return respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
  }

  template <typename A1>
  aid_t respond(resp_t res, A1& a1, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (sender)
    {
      msg >> a1;
    }
    return sender;
  }

  template <typename A1, typename A2>
  aid_t respond(resp_t res, A1& a1, A2& a2, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (sender)
    {
      msg >> a1 >> a2;
    }
    return sender;
  }

  template <typename A1, typename A2, typename A3>
  aid_t respond(resp_t res, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (sender)
    {
      msg >> a1 >> a2 >> a3;
    }
    return sender;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  aid_t respond(resp_t res, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (sender)
    {
      msg >> a1 >> a2 >> a3 >> a4;
    }
    return sender;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t respond(resp_t res, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (sender)
    {
      msg >> a1 >> a2 >> a3 >> a4 >> a5;
    }
    return sender;
  }
};

template <typename ActorRef>
struct actor_wrap<ActorRef, true>
  : public sender<ActorRef>
{
  typedef sender<ActorRef> base_t;
  typedef ActorRef actor_ref_t;

  void recv(aid_t& sender, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = begin_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv0<actor_ref_t>, _1, _2, _3,
        boost::ref(sender), has_exit
        ),
      patt
      );
  }

  template <typename Match, typename A1>
  void recv(aid_t& sender, Match type, A1& a1, duration_t tmo = infin)
  {
    pattern patt(tmo);
    make_pattern(patt, type);
    bool has_exit = begin_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv1<actor_ref_t, A1>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1)
        ),
      patt
      );
  }

  template <typename Match, typename A1, typename A2>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, duration_t tmo = infin)
  {
    pattern patt(tmo);
    make_pattern(patt, type);
    bool has_exit = begin_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv2<actor_ref_t, A1, A2>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2)
        ),
      patt
      );
  }

  template <typename Match, typename A1, typename A2, typename A3>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    pattern patt(tmo);
    make_pattern(patt, type);
    bool has_exit = begin_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv3<actor_ref_t, A1, A2, A3>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2),
        boost::ref(a3)
        ),
      patt
      );
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    pattern patt(tmo);
    make_pattern(patt, type);
    bool has_exit = begin_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv4<actor_ref_t, A1, A2, A3, A4>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2),
        boost::ref(a3), boost::ref(a4)
        ),
      patt
      );
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    pattern patt(tmo);
    make_pattern(patt, type);
    bool has_exit = begin_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv5<actor_ref_t, A1, A2, A3, A4, A5>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2),
        boost::ref(a3), boost::ref(a4), boost::ref(a5)
        ),
      patt
      );
  }

  void respond(resp_t res, aid_t& sender, duration_t tmo = infin)
  {
    pattern patt(tmo);
    actor_ref_t& a = base_t::get_actor_ref();
    a->respond(
      boost::bind(
        &handle_recv0<actor_ref_t>, _1, _2, _3,
        boost::ref(sender), false
        ),
      res, tmo
      );
  }

  template <typename A1>
  void respond(resp_t res, aid_t& sender, A1& a1, duration_t tmo = infin)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a->respond(
      boost::bind(
        &handle_recv1<actor_ref_t, A1>, _1, _2, _3,
        boost::ref(sender), false, boost::ref(a1)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2>
  void respond(resp_t res, aid_t& sender, A1& a1, A2& a2, duration_t tmo = infin)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a->respond(
      boost::bind(
        &handle_recv2<actor_ref_t, A1, A2>, _1, _2, _3,
        boost::ref(sender), false, boost::ref(a1), boost::ref(a2)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2, typename A3>
  void respond(resp_t res, aid_t& sender, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a->respond(
      boost::bind(
        &handle_recv3<actor_ref_t, A1, A2, A3>, _1, _2, _3,
        boost::ref(sender), false, boost::ref(a1), boost::ref(a2),
        boost::ref(a3)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2, typename A3, typename A4>
  void respond(resp_t res, aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a->respond(
      boost::bind(
        &detail::handle_recv4<actor_ref_t, A1, A2, A3, A4>, _1, _2, _3,
        boost::ref(sender), false, boost::ref(a1), boost::ref(a2),
        boost::ref(a3), boost::ref(a4)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void respond(resp_t res, aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a->respond(
      boost::bind(
        &detail::handle_recv5<actor_ref_t, A1, A2, A3, A4, A5>, _1, _2, _3,
        boost::ref(sender), false, boost::ref(a1), boost::ref(a2),
        boost::ref(a3), boost::ref(a4), boost::ref(a5)
        ),
      res, tmo
      );
  }
};
}
}

#endif /// GCE_ACTOR_DETAIL_WRAP_HPP
