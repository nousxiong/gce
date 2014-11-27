///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SEND_HPP
#define GCE_ACTOR_DETAIL_SEND_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/message.hpp>

namespace gce
{
namespace detail
{
template <typename Sender, typename Recver>
inline void send_impl(Sender& sender, Recver const& recver, message& m)
{
  sender.send(recver, m);
}

template <typename Sender, typename Recver>
inline resp_t request_impl(Sender& sender, Recver const& recver, message& m)
{
  return sender.request(recver, m);
}

template <typename Sender>
inline void reply_impl(Sender& sender, aid_t recver, message& m)
{
  sender.reply(recver, m);
}

///----------------------------------------------------------------------------
/// Send
///----------------------------------------------------------------------------
template <typename Sender, typename Recver>
inline void send(Sender& sender, Recver const& recver)
{
  message m;
  send_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver>
inline void send(Sender& sender, Recver const& recver, match_t type)
{
  message m(type);
  send_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1>
inline void send(Sender& sender, Recver const& recver, match_t type, A1 const& a1)
{
  message m(type);
  m << a1;
  send_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1, typename A2>
inline void send(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2
  )
{
  message m(type);
  m << a1 << a2;
  send_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1, typename A2, typename A3>
inline void send(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3
  )
{
  message m(type);
  m << a1 << a2 << a3;
  send_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1, typename A2, typename A3, typename A4>
inline void send(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4;
  send_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <
  typename Sender, typename Recver, typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline void send(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4 << a5;
  send_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
/// Request
///----------------------------------------------------------------------------
template <typename Sender, typename Recver>
inline resp_t request(Sender& sender, Recver const& recver)
{
  message m;
  return request_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver>
inline resp_t request(Sender& sender, Recver const& recver, match_t type)
{
  message m(type);
  return request_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1>
inline resp_t request(Sender& sender, Recver const& recver, match_t type, A1 const& a1)
{
  message m(type);
  m << a1;
  return request_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1, typename A2>
inline resp_t request(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2
  )
{
  message m(type);
  m << a1 << a2;
  return request_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1, typename A2, typename A3>
inline resp_t request(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3
  )
{
  message m(type);
  m << a1 << a2 << a3;
  return request_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename Recver, typename A1, typename A2, typename A3, typename A4>
inline resp_t request(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4;
  return request_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <
  typename Sender, typename Recver, typename A1, typename A2,
  typename A3, typename A4, typename A5>
inline resp_t request(
  Sender& sender, Recver const& recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4 << a5;
  return request_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
/// Reply
///----------------------------------------------------------------------------
template <typename Sender>
inline void reply(Sender& sender, aid_t recver)
{
  message m;
  reply_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender>
inline void reply(Sender& sender, aid_t recver, match_t type)
{
  message m(type);
  reply_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1>
inline void reply(Sender& sender, aid_t recver, match_t type, A1 const& a1)
{
  message m(type);
  m << a1;
  reply_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2>
inline void reply(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2
  )
{
  message m(type);
  m << a1 << a2;
  reply_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2, typename A3>
inline void reply(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3
  )
{
  message m(type);
  m << a1 << a2 << a3;
  reply_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2, typename A3, typename A4>
inline void reply(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4;
  reply_impl(sender, recver, m);
}
///----------------------------------------------------------------------------
template <
  typename Sender, typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline void reply(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4 << a5;
  reply_impl(sender, recver, m);
}
}
}

#endif /// GCE_ACTOR_DETAIL_SEND_HPP
