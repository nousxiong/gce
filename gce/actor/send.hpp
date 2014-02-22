///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_SEND_HPP
#define GCE_ACTOR_SEND_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/slice.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/message.hpp>

namespace gce
{
namespace detail
{
template <typename Sender>
inline void send(Sender& sender, aid_t recver, message& m)
{
  sender.send(recver, m);
}

inline void send(slice_t& sender, aid_t recver, message& m)
{
  sender->send(recver, m);
}

template <typename Sender>
inline response_t request(Sender& sender, aid_t recver, message& m)
{
  return sender.request(recver, m);
}

inline response_t request(slice_t& sender, aid_t recver, message& m)
{
  return sender->request(recver, m);
}

template <typename Sender>
inline void reply(Sender& sender, aid_t recver, message& m)
{
  sender.reply(recver, m);
}

inline void reply(slice_t& sender, aid_t recver, message& m)
{
  sender->reply(recver, m);
}
}
///----------------------------------------------------------------------------
/// Send
///----------------------------------------------------------------------------
template <typename Sender>
inline void send(Sender& sender, aid_t recver)
{
  message m;
  detail::send(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender>
inline void send(Sender& sender, aid_t recver, match_t type)
{
  message m(type);
  detail::send(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1>
inline void send(Sender& sender, aid_t recver, match_t type, A1 const& a1)
{
  message m(type);
  m << a1;
  detail::send(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2>
inline void send(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2
  )
{
  message m(type);
  m << a1 << a2;
  detail::send(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2, typename A3>
inline void send(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3
  )
{
  message m(type);
  m << a1 << a2 << a3;
  detail::send(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2, typename A3, typename A4>
inline void send(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4;
  detail::send(sender, recver, m);
}
///----------------------------------------------------------------------------
template <
  typename Sender, typename A1, typename A2,
  typename A3, typename A4, typename A5
  >
inline void send(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4 << a5;
  detail::send(sender, recver, m);
}
///----------------------------------------------------------------------------
/// Request
///----------------------------------------------------------------------------
template <typename Sender>
inline response_t request(Sender& sender, aid_t recver)
{
  message m;
  return detail::request(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender>
inline response_t request(Sender& sender, aid_t recver, match_t type)
{
  message m(type);
  return detail::request(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1>
inline response_t request(Sender& sender, aid_t recver, match_t type, A1 const& a1)
{
  message m(type);
  m << a1;
  return detail::request(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2>
inline response_t request(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2
  )
{
  message m(type);
  m << a1 << a2;
  return detail::request(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2, typename A3>
inline response_t request(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3
  )
{
  message m(type);
  m << a1 << a2 << a3;
  return detail::request(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1, typename A2, typename A3, typename A4>
inline response_t request(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4;
  return detail::request(sender, recver, m);
}
///----------------------------------------------------------------------------
template <
  typename Sender, typename A1, typename A2,
  typename A3, typename A4, typename A5>
inline response_t request(
  Sender& sender, aid_t recver, match_t type,
  A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5
  )
{
  message m(type);
  m << a1 << a2 << a3 << a4 << a5;
  return detail::request(sender, recver, m);
}
///----------------------------------------------------------------------------
/// Reply
///----------------------------------------------------------------------------
template <typename Sender>
inline void reply(Sender& sender, aid_t recver)
{
  message m;
  detail::reply(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender>
inline void reply(Sender& sender, aid_t recver, match_t type)
{
  message m(type);
  detail::reply(sender, recver, m);
}
///----------------------------------------------------------------------------
template <typename Sender, typename A1>
inline void reply(Sender& sender, aid_t recver, match_t type, A1 const& a1)
{
  message m(type);
  m << a1;
  detail::reply(sender, recver, m);
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
  detail::reply(sender, recver, m);
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
  detail::reply(sender, recver, m);
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
  detail::reply(sender, recver, m);
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
  detail::reply(sender, recver, m);
}
///----------------------------------------------------------------------------
}

#endif /// GCE_ACTOR_SEND_HPP


