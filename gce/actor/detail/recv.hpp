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
#include <gce/actor/exception.hpp>
#include <gce/actor/to_match.hpp>
#include <boost/bind.hpp>
#include <algorithm>

namespace gce
{
namespace detail
{
template <typename Match>
inline void make_pattern(pattern& rt, Match type)
{
  rt.add_match(type);
}

inline void make_pattern(pattern& rt, match_list_t const& match_list)
{
  rt.match_list_.resize(rt.match_list_.size() + match_list.size());
  std::copy_backward(match_list.begin(), match_list.end(), rt.match_list_.end());
}

inline bool find_exit(match_t type)
{
  return type == exit;
}

inline bool check_exit(std::vector<match_t> const& match_list)
{
  if (match_list.empty())
  {
    return true;
  }
  else
  {
    std::vector<match_t>::const_iterator itr =
      std::find_if(
        match_list.begin(),
        match_list.end(),
        boost::bind(&find_exit, _arg1)
        );
    return itr != match_list.end();
  }
}

struct recv_meta
{
  recv_meta()
    : msg_(0)
    , matched_(0)
    , guard_(0)
    , tmo_(0)
  {
  }

  message* msg_;
  match_t* matched_;
  errcode_t* guard_;
  errcode_t* tmo_;
};

template <typename Tag, typename Recver>
inline std::pair<aid_t, bool> recv_impl(
  Tag, Recver& recver, message& msg, pattern& patt, recv_meta meta = recv_meta()
  )
{
  bool has_exit = check_exit(patt.match_list_);
  aid_t sender = recver.recv(msg, patt);
  if (meta.msg_ != 0)
  {
    *meta.msg_ = msg;
  }
  if (meta.matched_ != 0)
  {
    *meta.matched_ = msg.get_type();
  }
  if (!has_exit && msg.get_type() == exit)
  {
    if (meta.guard_ != 0)
    {
      *meta.guard_ = boost::asio::error::make_error_code(boost::asio::error::shut_down);
      return std::make_pair(sender, false);
    }
    else
    {
      exit_code_t exc;
      std::string errmsg;
      msg >> exc >> errmsg;
      if (exc == exit_normal)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::normal_exception").except<normal_exception>();
      }
      else if (exc == exit_except)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::runtime_exception").except<runtime_exception>();
      }
      else if (exc == exit_remote)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::remote_exception").except<remote_exception>();
      }
      else if (exc == exit_already)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::already_exit_exception").except<already_exit_exception>();
      }
      else if (exc == exit_neterr)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::neterr_exception").except<neterr_exception>();
      }
      else
      {
        GCE_ASSERT(false)(exc)(errmsg);
      }
    }
  }

  if (sender == aid_nil && meta.tmo_ != 0)
  {
    *meta.tmo_ = boost::asio::error::make_error_code(boost::asio::error::timed_out);
    return std::make_pair(sender, false);
  }

  GCE_VERIFY(sender != aid_nil)(msg)(patt)
    .msg("gce::recv_timeout_exception").except<recv_timeout_exception>();

  return std::make_pair(sender, true);
}

template <typename Recver>
inline std::pair<aid_t, bool> recv_impl(
  gce::nonblocked, Recver& recver, message& msg, pattern& patt, recv_meta meta = recv_meta()
  )
{
  bool has_exit = check_exit(patt.match_list_);
  aid_t sender = recver.recv(msg, patt.match_list_, patt.recver_);
  if (meta.msg_ != 0)
  {
    *meta.msg_ = msg;
  }
  if (meta.matched_ != 0)
  {
    *meta.matched_ = msg.get_type();
  }
  if (!has_exit && msg.get_type() == exit)
  {
    if (meta.guard_ != 0)
    {
      *meta.guard_ = boost::asio::error::make_error_code(boost::asio::error::shut_down);
      return std::make_pair(sender, false);
    }
    else
    {
      exit_code_t exc;
      std::string errmsg;
      msg >> exc >> errmsg;
      if (exc == exit_normal)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::normal_exception").except<normal_exception>();
      }
      else if (exc == exit_except)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::runtime_exception").except<runtime_exception>();
      }
      else if (exc == exit_remote)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::remote_exception").except<remote_exception>();
      }
      else if (exc == exit_already)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::already_exit_exception").except<already_exit_exception>();
      }
      else if (exc == exit_neterr)
      {
        GCE_VERIFY(false)(exc)(msg)(patt)(sender)
          .msg("gce::neterr_exception").except<neterr_exception>();
      }
      else
      {
        GCE_ASSERT(false)(exc)(errmsg);
      }
    }
  }
  return std::make_pair(sender, sender != aid_nil);
}

template <typename Tag, typename Recver>
inline std::pair<aid_t, bool> respond_impl(
  Tag, Recver& recver, resp_t res, message& msg, duration_t tmo, recv_meta meta = recv_meta()
  )
{
  aid_t sender = recver.respond(res, msg, tmo);
  if (meta.msg_ != 0)
  {
    *meta.msg_ = msg;
  }
  if (msg.get_type() == exit)
  {
    if (meta.guard_ != 0)
    {
      *meta.guard_ = boost::asio::error::make_error_code(boost::asio::error::shut_down);
      return std::make_pair(sender, false);
    }
    else
    {
      exit_code_t exc;
      std::string errmsg;
      msg >> exc >> errmsg;
      if (exc == exit_normal)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::normal_exception").except<normal_exception>();
      }
      else if (exc == exit_except)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::runtime_exception").except<runtime_exception>();
      }
      else if (exc == exit_remote)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::remote_exception").except<remote_exception>();
      }
      else if (exc == exit_already)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::already_exit_exception").except<already_exit_exception>();
      }
      else if (exc == exit_neterr)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::neterr_exception").except<neterr_exception>();
      }
      else
      {
        GCE_ASSERT(false)(exc)(errmsg);
      }
    }
  }

  if (sender == aid_nil && meta.tmo_ != 0)
  {
    *meta.tmo_ = boost::asio::error::make_error_code(boost::asio::error::timed_out);
    return std::make_pair(sender, false);
  }

  GCE_VERIFY(sender != aid_nil)(res)(msg)(tmo)
    .msg("gce::respond_timeout_exception").except<respond_timeout_exception>();
  return std::make_pair(sender, true);
}

template <typename Recver>
inline std::pair<aid_t, bool> respond_impl(
  gce::nonblocked, Recver& recver, resp_t res, message& msg, duration_t tmo, recv_meta meta = recv_meta()
  )
{
  aid_t sender = recver.respond(res, msg);
  if (meta.msg_ != 0)
  {
    *meta.msg_ = msg;
  }
  if (msg.get_type() == exit)
  {
    if (meta.guard_ != 0)
    {
      *meta.guard_ = boost::asio::error::make_error_code(boost::asio::error::shut_down);
      return std::make_pair(sender, false);
    }
    else
    {
      exit_code_t exc;
      std::string errmsg;
      msg >> exc >> errmsg;
      if (exc == exit_normal)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::normal_exception").except<normal_exception>();
      }
      else if (exc == exit_except)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::runtime_exception").except<runtime_exception>();
      }
      else if (exc == exit_remote)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::remote_exception").except<remote_exception>();
      }
      else if (exc == exit_already)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::already_exit_exception").except<already_exit_exception>();
      }
      else if (exc == exit_neterr)
      {
        GCE_VERIFY(false)(exc)(res)(msg)(sender)(tmo)
          .msg("gce::neterr_exception").except<neterr_exception>();
      }
      else
      {
        GCE_ASSERT(false)(exc)(errmsg);
      }
    }
  }
  return std::make_pair(sender, sender != aid_nil);
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
  else if (sender == aid_nil)
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
