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


static match_t default_ = make_match(0);

template <typename ActorRef>
struct actor_wrap<ActorRef, false>
  : public sender<ActorRef>
{
  typedef sender<ActorRef> base_t;
  typedef ActorRef actor_ref_t;
  typedef receiver<actor_ref_t, false> receiver_t;

  aid_t recv(duration_t tmo = infin)
  {
    return pri_recv(gce::guard(), tmo).first;
  }

  aid_t recv(gce::guard g, duration_t tmo = infin)
  {
    return pri_recv(g, tmo).first;
  }

  template <typename Match>
  aid_t recv(Match type, duration_t tmo = infin)
  {
    return pri_recv(type, gce::guard(), tmo).first;
  }
  
  template <typename Match>
  aid_t recv(Match type, gce::guard g, duration_t tmo = infin)
  {
    return pri_recv(type, g, tmo).first;
  }

  template <typename Match, typename A1>
  aid_t recv(Match type, A1& a1, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, gce::guard(), tmo, msg);
    if (pr.second)
    {
      msg >> a1;
    }
    return pr.first;
  }

  template <typename Match, typename A1>
  aid_t recv(Match type, gce::guard g, A1& a1, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, g, tmo, msg);
    if (pr.second)
    {
      msg >> a1;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2>
  aid_t recv(Match type, A1& a1, A2& a2, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, gce::guard(), tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, g, tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2, typename A3>
  aid_t recv(Match type, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, gce::guard(), tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2, typename A3>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, g, tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  aid_t recv(Match type, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, gce::guard(), tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3 >> a4;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, g, tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3 >> a4;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t recv(Match type, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, gce::guard(), tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3 >> a4 >> a5;
    }
    return pr.first;
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t recv(Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = pri_recv(type, g, tmo, msg);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3 >> a4 >> a5;
    }
    return pr.first;
  }

  aid_t respond(resp_t res, duration_t tmo = infin)
  {
    message msg;
    return respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo).first;
  }

  template <typename A1>
  aid_t respond(resp_t res, A1& a1, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (pr.second)
    {
      msg >> a1;
    }
    return pr.first;
  }

  template <typename A1, typename A2>
  aid_t respond(resp_t res, A1& a1, A2& a2, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (pr.second)
    {
      msg >> a1 >> a2;
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3>
  aid_t respond(resp_t res, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3;
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  aid_t respond(resp_t res, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3 >> a4;
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t respond(resp_t res, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    message msg;
    std::pair<aid_t, bool> pr = respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo);
    if (pr.second)
    {
      msg >> a1 >> a2 >> a3 >> a4 >> a5;
    }
    return pr.first;
  }

  receiver_t& match(match_t& matched = default_)
  {
    if (&matched != &default_)
    {
      rcv_.set_match(matched);
    }
    return rcv_;
  }

  receiver_t& match(match_list_t const& match_list, match_t& matched = default_)
  {
    BOOST_FOREACH(match_t const& mt, match_list)
    {
      rcv_.add_match(mt);
    }
    if (&matched != &default_)
    {
      rcv_.set_match(matched);
    }
    return rcv_;
  }

  template <typename Match1>
  receiver_t& match(Match1 type1, match_t& matched = default_)
  {
    rcv_.add_match(type1);
    if (&matched != &default_)
    {
      rcv_.set_match(matched);
    }
    return rcv_;
  }

  template <typename Match1, typename Match2>
  receiver_t& match(Match1 type1, Match2 type2, match_t& matched = default_)
  {
    rcv_.add_match(type1);
    rcv_.add_match(type2);
    if (&matched != &default_)
    {
      rcv_.set_match(matched);
    }
    return rcv_;
  }

  template <typename Match1, typename Match2, typename Match3>
  receiver_t& match(Match1 type1, Match2 type2, Match3 type3, match_t& matched = default_)
  {
    rcv_.add_match(type1);
    rcv_.add_match(type2);
    rcv_.add_match(type3);
    if (&matched != &default_)
    {
      rcv_.set_match(matched);
    }
    return rcv_;
  }

  template <typename Match1, typename Match2, typename Match3, typename Match4>
  receiver_t& match(Match1 type1, Match2 type2, Match3 type3, Match4 type4, match_t& matched = default_)
  {
    rcv_.add_match(type1);
    rcv_.add_match(type2);
    rcv_.add_match(type3);
    rcv_.add_match(type4);
    if (&matched != &default_)
    {
      rcv_.set_match(matched);
    }
    return rcv_;
  }

  template <typename Match1, typename Match2, typename Match3, typename Match4, typename Match5>
  receiver_t& match(Match1 type1, Match2 type2, Match3 type3, Match4 type4, Match5 type5, match_t& matched = default_)
  {
    rcv_.add_match(type1);
    rcv_.add_match(type2);
    rcv_.add_match(type3);
    rcv_.add_match(type4);
    rcv_.add_match(type5);
    if (&matched != &default_)
    {
      rcv_.set_match(matched);
    }
    return rcv_;
  }

  receiver_t& match(resp_t res)
  {
    rcv_.set_response(res);
    return rcv_;
  }

  receiver_t& timeout(duration_t tmo)
  {
    rcv_.timeout(tmo);
    return rcv_;
  }

  receiver_t& timeout(duration_t tmo, errcode_t& ec)
  {
    rcv_.timeout(tmo, ec);
    return rcv_;
  }

  template <typename Recver>
  receiver_t& guard(Recver const& recver)
  {
    rcv_.guard(recver);
    return rcv_;
  }

  template <typename Recver>
  receiver_t& guard(Recver const& recver, errcode_t& ec)
  {
    rcv_.guard(recver, ec);
    return rcv_;
  }

  receiver_t& guard(errcode_t& ec)
  {
    rcv_.guard(ec);
    return rcv_;
  }

  receiver_t& raw(message& msg)
  {
    rcv_.raw(msg);
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
  std::pair<aid_t, bool> pri_recv(Match type, gce::guard g, duration_t tmo, message& msg, bool has_match = true)
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
  std::pair<aid_t, bool> pri_recv(Match type, gce::guard g, duration_t tmo)
  {
    message msg;
    return pri_recv(type, g, tmo, msg);
  }

  std::pair<aid_t, bool> pri_recv(gce::guard g, duration_t tmo)
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

  template <typename Match>
  void recv(aid_t& sender, Match type, duration_t tmo = infin)
  {
    pri_recv0(sender, type, gce::guard(), tmo);
  }

  template <typename Match>
  void recv(aid_t& sender, Match type, gce::guard g, duration_t tmo = infin)
  {
    pri_recv0(sender, type, g, tmo);
  }

private:
  template <typename Match>
  void pri_recv0(aid_t& sender, Match type, gce::guard g, duration_t tmo, bool has_match = true)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, gce::guard(), has_match);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv0<actor_ref_t>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr
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
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, gce::guard());
    pri_recv1(sender, pr, patt, a1);
  }

  template <typename Match, typename A1>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, duration_t tmo = infin)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, g);
    pri_recv1(sender, pr, patt, a1);
  }

private:
  template <typename A1>
  void pri_recv1(aid_t& sender, std::pair<recv_meta, bool> pr, pattern& patt, A1& a1)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv1<actor_ref_t, A1>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr, boost::ref(a1)
        ),
      patt
      );
  }

public:
  template <typename Match, typename A1, typename A2>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, duration_t tmo = infin)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, gce::guard());
    pri_recv2(sender, pr, patt, a1, a2);
  }
  
  template <typename Match, typename A1, typename A2>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, duration_t tmo = infin)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, g);
    pri_recv2(sender, pr, patt, a1, a2);
  }

private:
  template <typename A1, typename A2>
  void pri_recv2(aid_t& sender, std::pair<recv_meta, bool> pr, pattern& patt, A1& a1, A2& a2)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv2<actor_ref_t, A1, A2>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr, boost::ref(a1), boost::ref(a2)
        ),
      patt
      );
  }

public:
  template <typename Match, typename A1, typename A2, typename A3>
  void recv(aid_t& sender, Match type, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, gce::guard());
    pri_recv3(sender, pr, patt, a1, a2, a3);
  }

  template <typename Match, typename A1, typename A2, typename A3>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, g);
    pri_recv3(sender, pr, patt, a1, a2, a3);
  }

private:
  template <typename A1, typename A2, typename A3>
  void pri_recv3(aid_t& sender, std::pair<recv_meta, bool> pr, pattern& patt, A1& a1, A2& a2, A3& a3)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv3<actor_ref_t, A1, A2, A3>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr, boost::ref(a1), boost::ref(a2), 
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
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, gce::guard());
    pri_recv4(sender, pr, patt, a1, a2, a3, a4);
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, g);
    pri_recv4(sender, pr, patt, a1, a2, a3, a4);
  }

private:
  template <typename A1, typename A2, typename A3, typename A4>
  void pri_recv4(aid_t& sender, std::pair<recv_meta, bool> pr, pattern& patt, A1& a1, A2& a2, A3& a3, A4& a4)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv4<actor_ref_t, A1, A2, A3, A4>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr, boost::ref(a1), boost::ref(a2), 
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
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, gce::guard());
    pri_recv5(sender, pr, patt, a1, a2, a3, a4, a5);
  }

  template <typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  void recv(aid_t& sender, Match type, gce::guard g, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin)
  {
    pattern patt(tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt, type, g);
    pri_recv5(sender, pr, patt, a1, a2, a3, a4, a5);
  }

private:
  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void pri_recv5(aid_t& sender, std::pair<recv_meta, bool> pr, pattern& patt, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
  {
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv5<actor_ref_t, A1, A2, A3, A4, A5>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr, boost::ref(a1), boost::ref(a2), 
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
        &handle_respond0<actor_ref_t>, _arg1, _arg2, _arg3,
        boost::ref(sender), recv_meta()
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
        &handle_respond1<actor_ref_t, A1>, _arg1, _arg2, _arg3,
        boost::ref(sender), recv_meta(), boost::ref(a1)
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
        &handle_respond2<actor_ref_t, A1, A2>, _arg1, _arg2, _arg3,
        boost::ref(sender), recv_meta(), boost::ref(a1), boost::ref(a2)
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
        &handle_respond3<actor_ref_t, A1, A2, A3>, _arg1, _arg2, _arg3,
        boost::ref(sender), recv_meta(), boost::ref(a1), boost::ref(a2),
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
        &handle_respond4<actor_ref_t, A1, A2, A3, A4>, _arg1, _arg2, _arg3,
        boost::ref(sender), recv_meta(), boost::ref(a1), boost::ref(a2),
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
        &handle_respond5<actor_ref_t, A1, A2, A3, A4, A5>, _arg1, _arg2, _arg3,
        boost::ref(sender), recv_meta(), boost::ref(a1), boost::ref(a2),
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
  std::pair<recv_meta, bool> pri_recv(pattern& patt, Match type, gce::guard g, bool has_match = true)
  {
    patt.recver_ = g.recver_;
    if (has_match)
    {
      make_pattern(patt, type);
    }
    return std::make_pair(recv_meta(), begin_recv(patt));
  }
};
}
}

#endif /// GCE_ACTOR_DETAIL_WRAP_HPP
