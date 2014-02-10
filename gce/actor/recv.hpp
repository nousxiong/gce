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

namespace gce
{
namespace detail
{
template <typename Recver>
inline aid_t recv(Recver& recver, message& msg, match& mach)
{
  return recver.recv(msg, mach);
}

inline aid_t recv(slice_t& recver, message& msg, match& mach)
{
  return recver->recv(msg, mach.match_list_);
}

template <typename Recver>
inline aid_t recv(Recver& recver, response_t res, message& msg, seconds_t tmo)
{
  return recver.recv(res, msg, tmo);
}

inline aid_t recv(slice_t& recver, response_t res, message& msg, seconds_t)
{
  return recver->recv(res, msg);
}
}
///----------------------------------------------------------------------------
/// Receive
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, seconds_t tmo = infin)
{
  message msg;
  match mach(tmo);
  return detail::recv(recver, msg, mach);
}
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, match_t type, seconds_t tmo = infin)
{
  message msg;
  match mach(tmo);
  mach.match_list_.push_back(type);
  return detail::recv(recver, msg, mach);
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1>
inline aid_t recv(Recver& recver, match_t type, A1& a1, seconds_t tmo = infin)
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
inline aid_t recv(Recver& recver, match_t type, A1& a1, A2& a2, seconds_t tmo = infin)
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
/// Receive response
///----------------------------------------------------------------------------
template <typename Recver>
inline aid_t recv(Recver& recver, response_t res, seconds_t tmo = infin)
{
  message msg;
  return detail::recv(recver, res, msg, tmo);
}
///----------------------------------------------------------------------------
template <typename Recver, typename A1>
inline aid_t recv(Recver& recver, response_t res, A1& a1, seconds_t tmo = infin)
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
inline aid_t recv(Recver& recver, response_t res, A1& a1, A2& a2, seconds_t tmo = infin)
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
}

#endif /// GCE_ACTOR_RECV_HPP



