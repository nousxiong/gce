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
#include <gce/actor/guard.hpp>
#include <gce/actor/detail/receiver.hpp>
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
  typedef receiver<actor_ref_t, false> receiver_t;

  aid_t recv(duration_t tmo = infin)
  {
    return pri_recv(gce::guard(), tmo);
  }

  aid_t recv(gce::guard g, duration_t tmo = infin)
  {
    return pri_recv(g, tmo);
  }

  template <typename Match>
  aid_t recv(Match type, duration_t tmo = infin)
  {
    return pri_recv(type, gce::guard(), tmo);
  }
  
  template <typename Match>
  aid_t recv(Match type, gce::guard g, duration_t tmo = infin)
  {
    return pri_recv(type, g, tmo);
  }

  template <typename Match, typename A1>
  aid_t recv(Match type, A1& a1, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = pri_recv(type, gce::guard(), tmo, msg);
    if (sender != aid_nil)
    {
      msg >> a1;
    }
    return sender;
  }

  template <typename Match, typename A1>
  aid_t recv(Match type, gce::guard g, A1& a1, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = pri_recv(type, g, tmo, msg);
    if (sender != aid_nil)
    {
      msg >> a1;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2>
  aid_t recv(Match type, A1& a1, A2& a2, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = pri_recv(type, gce::guard(), tmo, msg);
    if (sender != aid_nil)
    {
      msg >> a1 >> a2;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = pri_recv(type, g, tmo, msg);
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
    aid_t sender = pri_recv(type, gce::guard(), tmo, msg);
    if (sender)
    {
      msg >> a1 >> a2 >> a3;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2, typename A3>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = pri_recv(type, g, tmo, msg);
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
    aid_t sender = pri_recv(type, gce::guard(), tmo, msg);
    if (sender)
    {
      msg >> a1 >> a2 >> a3 >> a4;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = pri_recv(type, g, tmo, msg);
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
    aid_t sender = pri_recv(type, gce::guard(), tmo, msg);
    if (sender)
    {
      msg >> a1 >> a2 >> a3 >> a4 >> a5;
    }
    return sender;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    message msg;
    aid_t sender = pri_recv(type, g, tmo, msg);
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

  template <typename Match>
  receiver_t& match(Match type)
  {
    rcv_.match(type);
    return rcv_;
  }

  template <typename Rep, typename Period>
  receiver_t& timeout(boost::chrono::duration<Rep, Period> tmo)
  {
    rcv_.timeout(tmo);
    return rcv_;
  }

  template <typename Recver>
  receiver_t& guard(Recver const& recver)
  {
    rcv_.guard(recver);
    return rcv_;
  }

  /// internal use
  inline void set_actor_ref(actor_ref_t& a)
  {
    base_t::set_actor_ref(a);
    rcv_.set_actor_ref(a);
  }

private:
  template <typename Match>
  aid_t pri_recv(Match type, gce::guard g, duration_t tmo, message& msg, bool has_match = true)
  {
    pattern patt(tmo);
    if (has_match)
    {
      make_pattern(patt, type);
    }
    patt.recver_ = g.recver_;
    return recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
  }

  template <typename Match>
  aid_t pri_recv(Match type, gce::guard g, duration_t tmo)
  {
    message msg;
    return pri_recv(type, g, tmo, msg);
  }

  aid_t pri_recv(gce::guard g, duration_t tmo)
  {
    message msg;
    return pri_recv(match_nil, g, tmo, msg, false);
  }

private:
  receiver_t rcv_;
};

template <typename ActorRef>
struct actor_wrap<ActorRef, true>
  : public sender<ActorRef>
{
  typedef sender<ActorRef> base_t;
  typedef ActorRef actor_ref_t;

  void recv(aid_t& sender, duration_t tmo = infin)
  {
    pri_recv0(sender, gce::guard(), tmo);
  }

  void recv(aid_t& sender, gce::guard g, duration_t tmo = infin)
  {
    pri_recv0(sender, g, tmo);
  }

private:
  template <typename Match>
  void pri_recv0(aid_t& sender, Match type, gce::guard g, duration_t tmo, bool has_match = true)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, gce::guard());
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv0<actor_ref_t>, _1, _2, _3,
        boost::ref(sender), has_exit
        ),
      patt
      );
  }

  void pri_recv0(aid_t& sender, gce::guard g, duration_t tmo)
  {
    pri_recv0(sender, match_nil, g, tmo, false);
  }

public:
  template <typename Match, typename A1>
  void recv(aid_t& sender, Match type, A1& a1, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, gce::guard());
    pri_recv1(sender, has_exit, patt, a1);
  }

  template <typename Match, typename A1>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, g);
    pri_recv1(sender, has_exit, patt, a1);
  }

private:
  template <typename A1>
  void pri_recv1(aid_t& sender, bool has_exit, pattern& patt, A1& a1)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv1<actor_ref_t, A1>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1)
        ),
      patt
      );
  }

public:
  template <typename Match, typename A1, typename A2>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, gce::guard());
    pri_recv2(sender, has_exit, patt, a1, a2);
  }
  
  template <typename Match, typename A1, typename A2>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, g);
    pri_recv2(sender, has_exit, patt, a1, a2);
  }

private:
  template <typename A1, typename A2>
  void pri_recv2(aid_t& sender, bool has_exit, pattern& patt, A1& a1, A2& a2)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv2<actor_ref_t, A1, A2>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2)
        ),
      patt
      );
  }

public:
  template <typename Match, typename A1, typename A2, typename A3>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, gce::guard());
    pri_recv3(sender, has_exit, patt, a1, a2, a3);
  }

  template <typename Match, typename A1, typename A2, typename A3>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, g);
    pri_recv3(sender, has_exit, patt, a1, a2, a3);
  }

private:
  template <typename A1, typename A2, typename A3>
  void pri_recv3(aid_t& sender, bool has_exit, pattern& patt, A1& a1, A2& a2, A3& a3)
  {
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

public:
  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, gce::guard());
    pri_recv4(sender, has_exit, patt, a1, a2, a3, a4);
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, g);
    pri_recv4(sender, has_exit, patt, a1, a2, a3, a4);
  }

private:
  template <typename A1, typename A2, typename A3, typename A4>
  void pri_recv4(aid_t& sender, bool has_exit, pattern& patt, A1& a1, A2& a2, A3& a3, A4& a4)
  {
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

public:
  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, gce::guard());
    pri_recv5(sender, has_exit, patt, a1, a2, a3, a4, a5);
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt, type, g);
    pri_recv5(sender, has_exit, patt, a1, a2, a3, a4, a5);
  }

private:
  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void pri_recv5(aid_t& sender, bool has_exit, pattern& patt, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
  {
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

public:
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

  /// internal use
  inline void set_actor_ref(actor_ref_t& a)
  {
    base_t::set_actor_ref(a);
  }

private:
  template <typename Match>
  bool pri_recv(pattern& patt, Match type, gce::guard g, bool has_match = true)
  {
    patt.recver_ = g.recver_;
    if (has_match)
    {
      make_pattern(patt, type);
    }
    return begin_recv(patt);
  }
};
}
}

#endif /// GCE_ACTOR_DETAIL_WRAP_HPP
