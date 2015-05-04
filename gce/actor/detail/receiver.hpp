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
#include <utility>

namespace gce
{
namespace detail
{
template <typename ActorRef, typename SubType>
struct basic_receiver
{
  typedef ActorRef actor_ref_t;
  typedef SubType sub_t;

  sub_t& timeout(duration_t tmo)
  {
    tmo_ = tmo;
    return (sub_t&)*this;
  }

  sub_t& timeout(duration_t tmo, errcode_t& ec)
  {
    tmo_ = tmo;
    meta_.tmo_ = &ec;
    return (sub_t&)*this;
  }

  template <typename Recver>
  sub_t& guard(Recver const& recver)
  {
    g_.recver_ = recver;
    return (sub_t&)*this;
  }

  template <typename Recver>
  sub_t& guard(Recver const& recver, errcode_t& ec)
  {
    g_.recver_ = recver;
    meta_.guard_ = &ec;
    return (sub_t&)*this;
  }

  sub_t& guard(errcode_t& ec)
  {
    meta_.guard_ = &ec;
    return (sub_t&)*this;
  }

  sub_t& raw(message& msg)
  {
    meta_.msg_ = &msg;
    return (sub_t&)*this;
  }

  /// internal use
  basic_receiver()
    : a_(0)
    , has_match_(false)
    , tmo_(gce::infin)
  {
  }

  inline void set_actor_ref(actor_ref_t& a)
  {
    a_ = &a;
  }

  template <typename Match>
  void add_match(Match type)
  {
    match_list_.push_back(to_match(type));
    has_match_ = true;
  }

  void set_match(match_t& matched)
  {
    meta_.matched_ = &matched;
  }

  void set_response(resp_t res)
  {
    res_ = res;
    has_match_ = true;
  }

protected:
  inline void reset()
  {
    match_list_.clear();
    res_ = resp_t();
    has_match_ = false;
    tmo_ = gce::infin;
    g_ = gce::guard();
    meta_ = recv_meta();
  }

  inline actor_ref_t& get_actor_ref()
  {
    GCE_ASSERT(a_);
    return *a_;
  }

protected:
  actor_ref_t* a_;

  match_list_t match_list_;
  resp_t res_;
  bool has_match_;
  duration_t tmo_;
  gce::guard g_;

  recv_meta meta_;
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
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_recv(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    return pr.first;
  }

  template <typename A1>
  aid_t recv(A1& a1)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_recv(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1;
      }
      else
      {
        msg >> a1;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2>
  aid_t recv(A1& a1, A2& a2)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_recv(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2;
      }
      else
      {
        msg >> a1 >> a2;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3>
  aid_t recv(A1& a1, A2& a2, A3& a3)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_recv(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2 >> a3;
      }
      else
      {
        msg >> a1 >> a2 >> a3;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  aid_t recv(A1& a1, A2& a2, A3& a3, A4& a4)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_recv(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2 >> a3 >> a4;
      }
      else
      {
        msg >> a1 >> a2 >> a3 >> a4;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t recv(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_recv(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2 >> a3 >> a4 >> a5;
      }
      else
      {
        msg >> a1 >> a2 >> a3 >> a4 >> a5;
      }
    }
    return pr.first;
  }

  aid_t respond()
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_respond(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    return pr.first;
  }

  template <typename A1>
  aid_t respond(A1& a1)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_respond(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1;
      }
      else
      {
        msg >> a1;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2>
  aid_t respond(A1& a1, A2& a2)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_respond(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2;
      }
      else
      {
        msg >> a1 >> a2;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3>
  aid_t respond(A1& a1, A2& a2, A3& a3)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_respond(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2 >> a3;
      }
      else
      {
        msg >> a1 >> a2 >> a3;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  aid_t respond(A1& a1, A2& a2, A3& a3, A4& a4)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_respond(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2 >> a3 >> a4;
      }
      else
      {
        msg >> a1 >> a2 >> a3 >> a4;
      }
    }
    return pr.first;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  aid_t respond(A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
  {
    message msg;
    message* meta_msg = base_t::meta_.msg_;
    std::pair<aid_t, bool> pr = pri_respond(msg);
    if (meta_msg)
    {
      *meta_msg = msg;
    }
    if (pr.second)
    {
      if (meta_msg)
      {
        *meta_msg >> a1 >> a2 >> a3 >> a4 >> a5;
      }
      else
      {
        msg >> a1 >> a2 >> a3 >> a4 >> a5;
      }
    }
    return pr.first;
  }

  /// internal use
  receiver() {}

private:
  std::pair<aid_t, bool> pri_recv(message& msg)
  {
    pattern patt(base_t::tmo_);
    if (base_t::has_match_)
    {
      make_pattern(patt, base_t::match_list_);
    }
    patt.recver_ = base_t::g_.recver_;
    recv_meta meta = base_t::meta_;
    base_t::reset();
    return recv_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), msg, patt, meta);
  }

  
  std::pair<aid_t, bool> pri_respond(message& msg)
  {
    resp_t res = base_t::res_;
    duration_t tmo = base_t::tmo_;
    recv_meta meta = base_t::meta_;
    base_t::reset();
    return respond_impl(typename actor_ref_t::type(), base_t::get_actor_ref(), res, msg, tmo, meta);
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
    pattern patt(base_t::tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv0<actor_ref_t>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr
        ),
      patt
      );
  }

  template <typename A1>
  void recv(aid_t& sender, A1& a1)
  {
    pattern patt(base_t::tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv1<actor_ref_t, A1>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr, boost::ref(a1)
        ),
      patt
      );
  }

  template <typename A1, typename A2>
  void recv(aid_t& sender, A1& a1, A2& a2)
  {
    pattern patt(base_t::tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt);
    actor_ref_t& a = base_t::get_actor_ref();
    a.recv(
      boost::bind(
        &handle_recv2<actor_ref_t, A1, A2>, _arg1, _arg2, _arg3,
        boost::ref(sender), pr, boost::ref(a1), boost::ref(a2)
        ),
      patt
      );
  }

  template <typename A1, typename A2, typename A3>
  void recv(aid_t& sender, A1& a1, A2& a2, A3& a3)
  {
    pattern patt(base_t::tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt);
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

  template <typename A1, typename A2, typename A3, typename A4>
  void recv(aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4)
  {
    pattern patt(base_t::tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt);
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

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void recv(aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
  {
    pattern patt(base_t::tmo);
    std::pair<recv_meta, bool> pr = pri_recv(patt);
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

  void respond(aid_t& sender)
  {
    resp_t res = base_t::res_;
    duration_t tmo = base_t::tmo;
    recv_meta meta = pri_respond();
    actor_ref_t& a = base_t::get_actor_ref();
    a.respond(
      boost::bind(
        &handle_respond0<actor_ref_t>, _arg1, _arg2, _arg3,
        boost::ref(sender), meta
        ),
      res, tmo
      );
  }

  template <typename A1>
  void respond(aid_t& sender, A1& a1)
  {
    resp_t res = base_t::res_;
    duration_t tmo = base_t::tmo;
    recv_meta meta = pri_respond();
    actor_ref_t& a = base_t::get_actor_ref();
    a.respond(
      boost::bind(
        &handle_respond1<actor_ref_t, A1>, _arg1, _arg2, _arg3,
        boost::ref(sender), meta, boost::ref(a1)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2>
  void respond(aid_t& sender, A1& a1, A2& a2)
  {
    resp_t res = base_t::res_;
    duration_t tmo = base_t::tmo;
    recv_meta meta = pri_respond();
    actor_ref_t& a = base_t::get_actor_ref();
    a.respond(
      boost::bind(
        &handle_respond2<actor_ref_t, A1, A2>, _arg1, _arg2, _arg3,
        boost::ref(sender), meta, boost::ref(a1), boost::ref(a2)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2, typename A3>
  void respond(aid_t& sender, A1& a1, A2& a2, A3& a3)
  {
    resp_t res = base_t::res_;
    duration_t tmo = base_t::tmo;
    recv_meta meta = pri_respond();
    actor_ref_t& a = base_t::get_actor_ref();
    a.respond(
      boost::bind(
        &handle_respond3<actor_ref_t, A1, A2, A3>, _arg1, _arg2, _arg3,
        boost::ref(sender), meta, boost::ref(a1), boost::ref(a2), 
        boost::ref(a3)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2, typename A3, typename A4>
  void respond(aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4)
  {
    resp_t res = base_t::res_;
    duration_t tmo = base_t::tmo;
    recv_meta meta = pri_respond();
    actor_ref_t& a = base_t::get_actor_ref();
    a.respond(
      boost::bind(
        &handle_respond4<actor_ref_t, A1, A2, A3, A4>, _arg1, _arg2, _arg3,
        boost::ref(sender), meta, boost::ref(a1), boost::ref(a2), 
        boost::ref(a3), boost::ref(a4)
        ),
      res, tmo
      );
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void respond(aid_t& sender, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5)
  {
    resp_t res = base_t::res_;
    duration_t tmo = base_t::tmo;
    recv_meta meta = pri_respond();
    actor_ref_t& a = base_t::get_actor_ref();
    a.respond(
      boost::bind(
        &handle_respond5<actor_ref_t, A1, A2, A3, A4, A5>, _arg1, _arg2, _arg3,
        boost::ref(sender), meta, boost::ref(a1), boost::ref(a2), 
        boost::ref(a3), boost::ref(a4), boost::ref(a5)
        ),
      res, tmo
      );
  }

  /// internal use
  receiver() {}

private:
  std::pair<recv_meta, bool> pri_recv(pattern& patt)
  {
    if (base_t::has_match_)
    {
      make_pattern(patt, base_t::match_list_);
    }
    patt.recver_ = base_t::g_.recver_;
    recv_meta meta = base_t::meta_;
    base_t::reset();
    return std::make_pair(meta, begin_recv(patt));
  }

  recv_meta pri_respond()
  {
    recv_meta meta = base_t::meta_;
    base_t::reset();
    return meta;
  }
};
}
}

#endif /// GCE_ACTOR_DETAIL_RECEIVER_HPP
