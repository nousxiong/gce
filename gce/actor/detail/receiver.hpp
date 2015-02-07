///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_RECEIVER_HPP
#define GCE_ACTOR_DETAIL_RECEIVER_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/guard.hpp>
#include <gce/actor/detail/recv.hpp>

namespace gce
{
namespace detail
{
template <typename ActorRef, typename SubType>
struct basic_receiver
{
  typedef ActorRef actor_ref_t;
  typedef SubType sub_t;

  template <typename Match>
  sub_t& match(Match type)
  {
    type_ = to_match(type);
    has_match_ = true;
    return (sub_t&)*this;
  }

  template <typename Rep, typename Period>
  sub_t& timeout(boost::chrono::duration<Rep, Period> tmo)
  {
    tmo_ = tmo;
    return (sub_t&)*this;
  }

  template <typename Recver>
  sub_t& guard(Recver const& recver)
  {
    g_.recver_ = recver;
    return (sub_t&)*this;
  }

  /// internal use
  basic_receiver()
    : a_(0)
    , type_(match_nil)
    , has_match_(false)
  {
  }

  inline void set_actor_ref(actor_ref_t& a)
  {
    a_ = &a;
  }

protected:
  inline void reset()
  {
    type_ = match_nil;
    has_match_ = false;
    tmo_ = duration_t();
    g_ = gce::guard();
  }

  inline actor_ref_t& get_actor_ref()
  {
    GCE_ASSERT(a_);
    return *a_;
  }

protected:
  actor_ref_t* a_;

  match_t type_;
  bool has_match_;
  duration_t tmo_;
  gce::guard g_;
};

template <typename ActorRef, bool is_stackless>
struct receiver
{
};

template <typename ActorRef>
struct receiver<ActorRef, false>
  : public basic_receiver<ActorRef, receiver<ActorRef, false> >
{
  typedef ActorRef actor_ref_t;
  typedef receiver<actor_ref_t, false> self_t;
  typedef basic_receiver<actor_ref_t, self_t> base_t;

  aid_t recv()
  {
    message msg;
    return pri_recv(msg);
  }

  template <typename A1>
  aid_t recv(A1& a1)
  {
    message msg;
    aid_t sender = pri_recv(msg);
    if (sender)
    {
      msg >> a1;
    }
    return sender;
  }

  template <typename A1, typename A2>
  aid_t recv(A1& a1, A2& a2)
  {
    message msg;
    aid_t sender = pri_recv(msg);
    if (sender)
    {
      msg >> a1 >> a2;
    }
    return sender;
  }

  /// internal use
  receiver() {}

private:
  aid_t pri_recv(message& msg)
  {
    pattern patt(base_t::tmo_);
    if (base_t::has_match_)
    {
      make_pattern(patt, base_t::type_);
    }
    patt.recver_ = base_t::g_.recver_;
    base_t::reset();
    return recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt);
  }
};

template <typename ActorRef>
struct receiver<ActorRef, true>
  : public basic_receiver<ActorRef, receiver<ActorRef, true> >
{
  typedef ActorRef actor_ref_t;
  typedef receiver<actor_ref_t, true> self_t;
  typedef basic_receiver<actor_ref_t, self_t> base_t;

  void recv(aid_t& sender)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv0<actor_ref_t>, _1, _2, _3,
        boost::ref(sender), has_exit
        ),
      patt
      );
  }

  template <typename A1>
  void recv(aid_t& sender, A1& a1)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv1<actor_ref_t, A1>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1)
        ),
      patt
      );
  }

  template <typename A1, typename A2>
  void recv(aid_t& sender, A1& a1, A2& a2)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv2<actor_ref_t, A1, A2>, _1, _2, _3,
        boost::ref(sender), has_exit, boost::ref(a1), boost::ref(a2)
        ),
      patt
      );
  }

  template <typename A1, typename A2, typename A3>
  void recv(aid_t& sender, A1& a1, A2& a2, A3& a3)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt);
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

  template <typename A1, typename A2, typename A3, typename A4>
  void recv(aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt);
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

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void recv(aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
  {
    pattern patt(tmo);
    bool has_exit = pri_recv(patt);
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

  /// internal use
  receiver() {}

private:
  bool pri_recv(pattern& patt)
  {
    patt.recver_ = base_t::g_.recver_;
    if (base_t::has_match_)
    {
      make_pattern(patt, base_t::type_);
    }
    base_t::reset();
    return begin_recv(patt);
  }
};
}
}

#endif /// GCE_ACTOR_DETAIL_RECEIVER_HPP
