///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_RECV_HPP
#define GCE_ACTOR_DETAIL_RECV_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/pattern.hpp>
#include <gce/actor/detail/to_match.hpp>
#include <boost/bind.hpp>
#include <algorithm>

namespace gce
{
namespace detail
{
template <typename Match>
inline void make_pattern(pattern& rt, Match type)
{
  rt.match_list_.push_back(to_match(type));
}

inline bool find_exit(match_t type)
{
  return type == exit;
}

inline bool check_exit(std::vector<match_t>& match_list)
{
  if (match_list.empty())
  {
    return true;
  }
  else
  {
    std::vector<match_t>::iterator itr =
      std::find_if(
        match_list.begin(),
        match_list.end(),
        boost::bind(&find_exit, _1)
        );
    return itr != match_list.end();
  }
}

template <typename Tag, typename Recver>
inline aid_t recv_impl(Tag, Recver& recver, message& msg, pattern& patt)
{
  bool has_exit = check_exit(patt.match_list_);
  aid_t sender = recver.recv(msg, patt);
  if (!has_exit && msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    GCE_VERIFY(false)(exc)(msg)(patt)(sender).msg(errmsg.c_str());
  }

  GCE_VERIFY(sender)(msg)(patt).msg("recv timeout");
  return sender;
}

template <typename Recver>
inline aid_t recv_impl(gce::nonblocked, Recver& recver, message& msg, pattern& patt)
{
  bool has_exit = check_exit(patt.match_list_);
  aid_t sender = recver.recv(msg, patt.match_list_, patt.recver_);
  if (!has_exit && msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    GCE_VERIFY(false)(exc)(msg)(patt)(sender).msg(errmsg.c_str());
  }
  return sender;
}

template <typename Tag, typename Recver>
inline aid_t respond_impl(Tag, Recver& recver, resp_t res, message& msg, duration_t tmo)
{
  aid_t sender = recver.respond(res, msg, tmo);
  if (msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo.count()).msg(errmsg.c_str());
  }

  GCE_VERIFY(sender)(res)(msg)(tmo.count()).msg("recv response timeout");
  return sender;
}

template <typename Recver>
inline aid_t respond_impl(gce::nonblocked, Recver& recver, resp_t res, message& msg, duration_t)
{
  aid_t sender = recver.respond(res, msg);
  if (msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    GCE_VERIFY(false)(exc)(res)(msg)(sender).msg(errmsg.c_str());
  }
  return sender;
}
///------------------------------------------------------------------------------
/// recv stackless
///------------------------------------------------------------------------------
inline bool begin_recv(pattern& patt)
{
  return check_exit(patt.match_list_);
}

template <typename Stackless>
inline bool end_recv(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, bool has_exit
  )
{
  osender = sender;
  bool ret = false;
  if (!has_exit && msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    recver.get_actor().quit(exc, errmsg);
  }
  else if (!sender)
  {
    recver.get_actor().quit(exit_except, "recv timeout");
  }
  else
  {
    ret = true;
  }
  return ret;
}

template <typename Stackless>
inline void handle_recv0(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, bool has_exit
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    recver.resume();
  }
}

template <typename Stackless, typename A1>
inline void handle_recv1(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, bool has_exit, A1& a1
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1;
    recver.resume();
  }
}

template <typename Stackless, typename A1, typename A2>
inline void handle_recv2(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, bool has_exit, A1& a1, A2& a2
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2;
    recver.resume();
  }
}

template <typename Stackless, typename A1, typename A2, typename A3>
inline void handle_recv3(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, bool has_exit, A1& a1, A2& a2, A3& a3
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2 >> a3;
    recver.resume();
  }
}

template <typename Stackless, typename A1, typename A2, typename A3, typename A4>
inline void handle_recv4(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, bool has_exit, A1& a1, A2& a2, A3& a3, A4& a4
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2 >> a3 >> a4;
    recver.resume();
  }
}

template <typename Stackless, typename A1, typename A2, typename A3, typename A4, typename A5>
inline void handle_recv5(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, bool has_exit, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5
  )
{
  if (end_recv(recver, sender, msg, osender, has_exit))
  {
    msg >> a1 >> a2 >> a3 >> a4 >> a5;
    recver.resume();
  }
}
}
}

#endif /// GCE_ACTOR_DETAIL_RECV_HPP
