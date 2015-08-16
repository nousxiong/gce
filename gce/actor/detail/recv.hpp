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
  rt.match_list_.reserve(rt.match_list_.size() + match_list.size());
  for (size_t i=0, size=match_list.size(); i<size; ++i)
  {
    rt.match_list_.push_back(match_list[i]);
  }
  //std::copy_backward(match_list.begin(), match_list.end(), rt.match_list_.end());
}

struct find_exit
{
  bool operator()(match_t type) const
  {
    return type == exit;
  }
};

template <typename MatchList>
inline bool check_exit(MatchList const& match_list)
{
  if (match_list.empty())
  {
    return true;
  }
  else
  {
    typename MatchList::const_iterator itr =
      std::find_if(match_list.begin(), match_list.end(), find_exit());
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

  if (!valid(sender) && meta.tmo_ != 0)
  {
    *meta.tmo_ = boost::asio::error::make_error_code(boost::asio::error::timed_out);
    return std::make_pair(sender, false);
  }

  GCE_VERIFY(valid(sender))(msg)(patt)
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
  return std::make_pair(sender, valid(sender));
}

template <typename Tag, typename Recver>
inline std::pair<aid_t, bool> respond_impl(
  Tag, Recver& recver, resp_t const& res, message& msg, duration_t tmo, recv_meta meta = recv_meta()
  )
{
  aid_t sender = recver.respond(res, msg, tmo);
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

  if (!valid(sender) && meta.tmo_ != 0)
  {
    *meta.tmo_ = boost::asio::error::make_error_code(boost::asio::error::timed_out);
    return std::make_pair(sender, false);
  }

  GCE_VERIFY(valid(sender))(res)(msg)(tmo)
    .msg("gce::respond_timeout_exception").except<respond_timeout_exception>();
  return std::make_pair(sender, true);
}

template <typename Recver>
inline std::pair<aid_t, bool> respond_impl(
  gce::nonblocked, Recver& recver, resp_t const& res, message& msg, duration_t tmo, recv_meta meta = recv_meta()
  )
{
  aid_t sender = recver.respond(res, msg);
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
  return std::make_pair(sender, valid(sender));
}
///------------------------------------------------------------------------------
/// recv stackless
///------------------------------------------------------------------------------
inline bool begin_recv(pattern& patt)
{
  return check_exit(patt.match_list_);
}

template <typename Stackless>
inline std::pair<bool, bool> end_recv(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, std::pair<recv_meta, bool> pr
  )
{
  osender = sender;
  if (!pr.second && msg.get_type() == exit)
  {
    if (pr.first.guard_ != 0)
    {
      *pr.first.guard_ = boost::asio::error::make_error_code(boost::asio::error::shut_down);
      return std::make_pair(false, true);
    }
    else
    {
      exit_code_t exc;
      std::string errmsg;
      msg >> exc >> errmsg;
      recver.get_actor().quit(exc, errmsg);
      return std::make_pair(false, false);
    }
  }
  else if (!valid(sender))
  {
    if (pr.first.tmo_ != 0)
    {
      *pr.first.tmo_ = boost::asio::error::make_error_code(boost::asio::error::timed_out);
      return std::make_pair(false, true);
    }
    else
    {
      recver.get_actor().quit(exit_except, "recv timeout");
      return std::make_pair(false, false);
    }
  }
  return std::make_pair(true, true);
}

template <typename Stackless>
inline std::pair<bool, bool> end_respond(
  Stackless recver, aid_t sender, message msg,
  aid_t& osender, recv_meta meta
  )
{
  osender = sender;
  if (msg.get_type() == exit)
  {
    if (meta.guard_ != 0)
    {
      *meta.guard_ = boost::asio::error::make_error_code(boost::asio::error::shut_down);
      return std::make_pair(false, true);
    }
    else
    {
      exit_code_t exc;
      std::string errmsg;
      msg >> exc >> errmsg;
      recver.get_actor().quit(exc, errmsg);
      return std::make_pair(false, false);
    }
  }
  else if (!valid(sender))
  {
    if (meta.tmo_ != 0)
    {
      *meta.tmo_ = boost::asio::error::make_error_code(boost::asio::error::timed_out);
      return std::make_pair(false, true);
    }
    else
    {
      recver.get_actor().quit(exit_except, "respond timeout");
      return std::make_pair(false, false);
    }
  }
  return std::make_pair(true, true);
}

//template <typename Stackless>
//inline void handle_recv0(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, std::pair<recv_meta, bool> pr
//  )
//{
//  std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender, pr);
//  message* meta_msg = pr.first.msg_;
//  if (meta_msg != 0)
//  {
//    *meta_msg = msg;
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

struct handle_recv0
{
  handle_recv0(aid_t& osender, std::pair<recv_meta, bool> const& pr)
    : osender_(osender)
    , pr_(pr)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender_, pr_);
    message* meta_msg = pr_.first.msg_;
    if (meta_msg != 0)
    {
      *meta_msg = msg;
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  std::pair<recv_meta, bool> pr_;
};

//template <typename Stackless, typename A1>
//inline void handle_recv1(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, std::pair<recv_meta, bool> pr, A1& a1
//  )
//{
  /*std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender, pr);
  message* meta_msg = pr.first.msg_;
  if (ret.first)
  {
    if (meta_msg != 0)
    {
      *meta_msg = msg;
      *meta_msg >> a1;
    }
    else
    {
      msg >> a1;
    }
  }
  if (ret.second)
  {
    recver.resume();
  }*/
//}

template <typename A1>
struct handle_recv1
{
  handle_recv1(
    aid_t& osender, std::pair<recv_meta, bool> const& pr,
    A1& a1
    )
    : osender_(osender)
    , pr_(pr)
    , a1_(a1)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender_, pr_);
    message* meta_msg = pr_.first.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_;
      }
      else
      {
        msg >> a1_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  std::pair<recv_meta, bool> pr_;
  A1& a1_;
};

//template <typename Stackless, typename A1, typename A2>
//inline void handle_recv2(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, std::pair<recv_meta, bool> pr, A1& a1, A2& a2
//  )
//{
//  std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender, pr);
//  message* meta_msg = pr.first.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2;
//    }
//    else
//    {
//      msg >> a1 >> a2;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2>
struct handle_recv2
{
  handle_recv2(
    aid_t& osender, std::pair<recv_meta, bool> const& pr,
    A1& a1, A2& a2
    )
    : osender_(osender)
    , pr_(pr)
    , a1_(a1)
    , a2_(a2)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender_, pr_);
    message* meta_msg = pr_.first.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_;
      }
      else
      {
        msg >> a1_ >> a2_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  std::pair<recv_meta, bool> pr_;
  A1& a1_;
  A2& a2_;
};

//template <typename Stackless, typename A1, typename A2, typename A3>
//inline void handle_recv3(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, std::pair<recv_meta, bool> pr, A1& a1, A2& a2, A3& a3
//  )
//{
//  std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender, pr);
//  message* meta_msg = pr.first.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2 >> a3;
//    }
//    else
//    {
//      msg >> a1 >> a2 >> a3;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2, typename A3>
struct handle_recv3
{
  handle_recv3(
    aid_t& osender, std::pair<recv_meta, bool> const& pr,
    A1& a1, A2& a2, A3& a3
    )
    : osender_(osender)
    , pr_(pr)
    , a1_(a1)
    , a2_(a2)
    , a3_(a3)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender_, pr_);
    message* meta_msg = pr_.first.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_ >> a3_;
      }
      else
      {
        msg >> a1_ >> a2_ >> a3_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  std::pair<recv_meta, bool> pr_;
  A1& a1_;
  A2& a2_;
  A3& a3_;
};

//template <typename Stackless, typename A1, typename A2, typename A3, typename A4>
//inline void handle_recv4(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, std::pair<recv_meta, bool> pr, A1& a1, A2& a2, A3& a3, A4& a4
//  )
//{
//  std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender, pr);
//  message* meta_msg = pr.first.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2 >> a3 >> a4;
//    }
//    else
//    {
//      msg >> a1 >> a2 >> a3 >> a4;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2, typename A3, typename A4>
struct handle_recv4
{
  handle_recv4(
    aid_t& osender, std::pair<recv_meta, bool> const& pr,
    A1& a1, A2& a2, A3& a3, A4& a4
    )
    : osender_(osender)
    , pr_(pr)
    , a1_(a1)
    , a2_(a2)
    , a3_(a3)
    , a4_(a4)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender_, pr_);
    message* meta_msg = pr_.first.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_ >> a3_ >> a4_;
      }
      else
      {
        msg >> a1_ >> a2_ >> a3_ >> a4_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  std::pair<recv_meta, bool> pr_;
  A1& a1_;
  A2& a2_;
  A3& a3_;
  A4& a4_;
};

//template <typename Stackless, typename A1, typename A2, typename A3, typename A4, typename A5>
//inline void handle_recv5(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, std::pair<recv_meta, bool> pr, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5
//  )
//{
//  std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender, pr);
//  message* meta_msg = pr.first.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2 >> a3 >> a4 >> a5;
//    }
//    else
//    {
//      msg >> a1 >> a2 >> a3 >> a4 >> a5;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
struct handle_recv5
{
  handle_recv5(
    aid_t& osender, std::pair<recv_meta, bool> const& pr,
    A1& a1, A2& a2, A3& a3, A4& a4, A5& a5
    )
    : osender_(osender)
    , pr_(pr)
    , a1_(a1)
    , a2_(a2)
    , a3_(a3)
    , a4_(a4)
    , a5_(a5)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_recv(recver, sender, msg, osender_, pr_);
    message* meta_msg = pr_.first.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_ >> a3_ >> a4_ >> a5_;
      }
      else
      {
        msg >> a1_ >> a2_ >> a3_ >> a4_ >> a5_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  std::pair<recv_meta, bool> pr_;
  A1& a1_;
  A2& a2_;
  A3& a3_;
  A4& a4_;
  A5& a5_;
};

//template <typename Stackless>
//inline void handle_respond0(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, recv_meta meta
//  )
//{
//  std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender, meta);
//  message* meta_msg = meta.msg_;
//  if (meta_msg != 0)
//  {
//    *meta_msg = msg;
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

struct handle_respond0
{
  handle_respond0(aid_t& osender, recv_meta const& meta)
    : osender_(osender)
    , meta_(meta)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender_, meta_);
    message* meta_msg = meta_.msg_;
    if (meta_msg != 0)
    {
      *meta_msg = msg;
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  recv_meta meta_;
};

//template <typename Stackless, typename A1>
//inline void handle_respond1(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, recv_meta meta, A1& a1
//  )
//{
//  std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender, meta);
//  message* meta_msg = meta.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1;
//    }
//    else
//    {
//      msg >> a1;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1>
struct handle_respond1
{
  handle_respond1(
    aid_t& osender, recv_meta const& meta, 
    A1& a1
    )
    : osender_(osender)
    , meta_(meta)
    , a1_(a1)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender_, meta_);
    message* meta_msg = meta_.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_;
      }
      else
      {
        msg >> a1_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  recv_meta meta_;
  A1& a1_;
};

//template <typename Stackless, typename A1, typename A2>
//inline void handle_respond2(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, recv_meta meta, A1& a1, A2& a2
//  )
//{
//  std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender, meta);
//  message* meta_msg = meta.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2;
//    }
//    else
//    {
//      msg >> a1 >> a2;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2>
struct handle_respond2
{
  handle_respond2(
    aid_t& osender, recv_meta const& meta, 
    A1& a1, A2& a2
    )
    : osender_(osender)
    , meta_(meta)
    , a1_(a1)
    , a2_(a2)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender_, meta_);
    message* meta_msg = meta_.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_;
      }
      else
      {
        msg >> a1_ >> a2_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  recv_meta meta_;
  A1& a1_;
  A2& a2_;
};

//template <typename Stackless, typename A1, typename A2, typename A3>
//inline void handle_respond3(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, recv_meta meta, A1& a1, A2& a2, A3& a3
//  )
//{
//  std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender, meta);
//  message* meta_msg = meta.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2 >> a3;
//    }
//    else
//    {
//      msg >> a1 >> a2 >> a3;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2, typename A3>
struct handle_respond3
{
  handle_respond3(
    aid_t& osender, recv_meta const& meta, 
    A1& a1, A2& a2, A3& a3
    )
    : osender_(osender)
    , meta_(meta)
    , a1_(a1)
    , a2_(a2)
    , a3_(a3)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender_, meta_);
    message* meta_msg = meta_.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_ >> a3_;
      }
      else
      {
        msg >> a1_ >> a2_ >> a3_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  recv_meta meta_;
  A1& a1_;
  A2& a2_;
  A3& a3_;
};

//template <typename Stackless, typename A1, typename A2, typename A3, typename A4>
//inline void handle_respond4(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, recv_meta meta, A1& a1, A2& a2, A3& a3, A4& a4
//  )
//{
//  std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender, meta);
//  message* meta_msg = meta.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2 >> a3 >> a4;
//    }
//    else
//    {
//      msg >> a1 >> a2 >> a3 >> a4;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2, typename A3, typename A4>
struct handle_respond4
{
  handle_respond4(
    aid_t& osender, recv_meta const& meta, 
    A1& a1, A2& a2, A3& a3, A4& a4
    )
    : osender_(osender)
    , meta_(meta)
    , a1_(a1)
    , a2_(a2)
    , a3_(a3)
    , a4_(a4)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender_, meta_);
    message* meta_msg = meta_.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_ >> a3_ >> a4_;
      }
      else
      {
        msg >> a1_ >> a2_ >> a3_ >> a4_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  recv_meta meta_;
  A1& a1_;
  A2& a2_;
  A3& a3_;
  A4& a4_;
};

//template <typename Stackless, typename A1, typename A2, typename A3, typename A4, typename A5>
//inline void handle_respond5(
//  Stackless recver, aid_t sender, message msg,
//  aid_t& osender, recv_meta meta, A1& a1, A2& a2, A3& a3, A4& a4, A5& a5
//  )
//{
//  std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender, meta);
//  message* meta_msg = meta.msg_;
//  if (ret.first)
//  {
//    if (meta_msg != 0)
//    {
//      *meta_msg = msg;
//      *meta_msg >> a1 >> a2 >> a3 >> a4 >> a5;
//    }
//    else
//    {
//      msg >> a1 >> a2 >> a3 >> a4 >> a5;
//    }
//  }
//  if (ret.second)
//  {
//    recver.resume();
//  }
//}

template <typename A1, typename A2, typename A3, typename A4, typename A5>
struct handle_respond5
{
  handle_respond5(
    aid_t& osender, recv_meta const& meta, 
    A1& a1, A2& a2, A3& a3, A4& a4, A5& a5
    )
    : osender_(osender)
    , meta_(meta)
    , a1_(a1)
    , a2_(a2)
    , a3_(a3)
    , a4_(a4)
    , a5_(a5)
  {
  }

  template <typename Stackless>
  void operator()(Stackless& recver, aid_t const& sender, message& msg) const
  {
    std::pair<bool, bool> ret = end_respond(recver, sender, msg, osender_, meta_);
    message* meta_msg = meta_.msg_;
    if (ret.first)
    {
      if (meta_msg != 0)
      {
        *meta_msg = msg;
        *meta_msg >> a1_ >> a2_ >> a3_ >> a4_ >> a5_;
      }
      else
      {
        msg >> a1_ >> a2_ >> a3_ >> a4_ >> a5_;
      }
    }
    if (ret.second)
    {
      recver.resume();
    }
  }

  aid_t& osender_;
  recv_meta meta_;
  A1& a1_;
  A2& a2_;
  A3& a3_;
  A4& a4_;
  A5& a5_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_RECV_HPP
