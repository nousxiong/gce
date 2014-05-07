///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_RECV_HPP
#define GCE_ACTOR_RECV_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/slice.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/match.hpp>
#include <boost/bind.hpp>
#include <algorithm>

namespace gce
{
namespace detail
{
inline bool find_exit(match_t type)
{
  return type == exit;
}

inline bool check_exit(std::vector<match_t>& match_list)
{
  if (match_list.empty())
  {
    return false;
  }
  else
  {
    std::vector<match_t>::iterator itr =
      std::find_if(
        match_list.begin(),
        match_list.end(),
        boost::bind(&find_exit, _1)
        );
    return itr == match_list.end();
  }
}

template <typename Recver>
inline aid_t recv(Recver& recver, message& msg, match& mach)
{
  bool add_exit = check_exit(mach.match_list_);
  if (add_exit)
  {
    mach.match_list_.push_back(exit);
  }

  aid_t sender = recver.recv(msg, mach);
  if (add_exit && msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }

  if (!sender)
  {
    throw std::runtime_error("recv timeout");
  }
  return sender;
}

inline aid_t recv(slice_t recver, message& msg, match& mach)
{
  bool has_exit = check_exit(mach.match_list_);
  if (!has_exit)
  {
    mach.match_list_.push_back(exit);
  }

  aid_t sender = recver.recv(msg, mach.match_list_);
  if (!has_exit && msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }
  return sender;
}

template <typename Recver>
inline aid_t recv(Recver& recver, response_t res, message& msg, duration_t tmo)
{
  aid_t sender = recver.recv(res, msg, tmo);
  if (msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }

  if (!sender)
  {
    throw std::runtime_error("recv response timeout");
  }
  return sender;
}

inline aid_t recv(slice_t recver, response_t res, message& msg, duration_t)
{
  aid_t sender = recver.recv(res, msg);
  if (msg.get_type() == exit)
  {
    exit_code_t exc;
    std::string errmsg;
    msg >> exc >> errmsg;
    throw std::runtime_error(errmsg);
  }
  return sender;
}
}
///----------------------------------------------------------------------------
/// Receive
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  return detail::recv(recver, msg, mach);
}
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, match_t type, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  return detail::recv(recver, msg, mach);
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1>
inline aid_t recv(Recver& recver, match_t type, A1& a1, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2>
inline aid_t recv(Recver& recver, match_t type, A1& a1, A2& a2, duration_t tmo = infin)
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3>
inline aid_t recv(
  Recver& recver, match_t type, A1& a1, A2& a2, A3& a3, duration_t tmo = infin
  )
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2 >> a3;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3, typename A4>
inline aid_t recv(
  Recver& recver, match_t type, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin
  )
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <
  typename Recver, typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline aid_t recv(
  Recver& recver, match_t type,
  A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin
  )
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  aid_t sender = detail::recv(recver, msg, mach);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4 >> a5;
  }
  return sender;
}
///----------------------------------------------------------------------------
/// Receive response
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, response_t res, duration_t tmo = infin)
{
  message msg;
  return detail::recv(recver, res, msg, tmo);
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1>
inline aid_t recv(Recver& recver, response_t res, A1& a1, duration_t tmo = infin)
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2>
inline aid_t recv(Recver& recver, response_t res, A1& a1, A2& a2, duration_t tmo = infin)
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3>
inline aid_t recv(Recver& recver, response_t res, A1& a1, A2& a2, A3& a3, duration_t tmo = infin)
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2 >> a3;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1, typename A2, typename A3, typename A4>
inline aid_t recv(
  Recver& recver, response_t res, A1& a1, A2& a2, A3& a3, A4& a4, duration_t tmo = infin
  )
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4;
  }
  return sender;
}
///----------------------------------------------------------------------------
template <
  typename Recver, typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline aid_t recv(
  Recver& recver, response_t res,
  A1& a1, A2& a2, A3& a3, A4& a4, A5& a5, duration_t tmo = infin
  )
{
  message msg;
  aid_t sender = detail::recv(recver, res, msg, tmo);
  if (sender)
  {
    msg >> a1 >> a2 >> a3 >> a4 >> a5;
  }
  return sender;
}
///----------------------------------------------------------------------------
}

#endif /// GCE_ACTOR_RECV_HPP



