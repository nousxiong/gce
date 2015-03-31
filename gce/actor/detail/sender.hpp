///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SENDER_HPP
#define GCE_ACTOR_DETAIL_SENDER_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/send.hpp>
#include <gce/actor/to_match.hpp>

namespace gce
{
namespace detail
{
template <typename ActorRef>
struct sender
{
  typedef ActorRef actor_ref_t;

  template <typename Recver>
  void send(Recver const& recver)
  {
    message m;
    send_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match>
  void send(Recver const& recver, Match type)
  {
    message m(to_match(type));
    send_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1>
  void send(Recver const& recver, Match type, A1 const& a1)
  {
    message m(to_match(type));
    m << a1;
    send_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2>
  void send(Recver const& recver, Match type, A1 const& a1, A2 const& a2)
  {
    message m(to_match(type));
    m << a1 << a2;
    send_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2, typename A3>
  void send(Recver const& recver, Match type, A1 const& a1, A2 const& a2, A3 const& a3)
  {
    message m(to_match(type));
    m << a1 << a2 << a3;
    send_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2, typename A3, typename A4>
  void send(Recver const& recver, Match type, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    message m(to_match(type));
    m << a1 << a2 << a3 << a4;
    send_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  void send(Recver const& recver, Match type, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    message m(to_match(type));
    m << a1 << a2 << a3 << a4 << a5;
    send_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver>
  resp_t request(Recver const& recver)
  {
    message m;
    return request_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match>
  resp_t request(Recver const& recver, Match type)
  {
    message m(to_match(type));
    return request_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1>
  resp_t request(Recver const& recver, Match type, A1 const& a1)
  {
    message m(to_match(type));
    m << a1;
    return request_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2>
  resp_t request(Recver const& recver, Match type, A1 const& a1, A2 const& a2)
  {
    message m(to_match(type));
    m << a1 << a2;
    return request_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2, typename A3>
  resp_t request(Recver const& recver, Match type, A1 const& a1, A2 const& a2, A3 const& a3)
  {
    message m(to_match(type));
    m << a1 << a2 << a3;
    return request_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2, typename A3, typename A4>
  resp_t request(Recver const& recver, Match type, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    message m(to_match(type));
    m << a1 << a2 << a3 << a4;
    return request_impl(get_actor_ref(), recver, m);
  }

  template <typename Recver, typename Match, typename A1, typename A2, typename A3, typename A4, typename A5>
  resp_t request(Recver const& recver, Match type, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    message m(to_match(type));
    m << a1 << a2 << a3 << a4 << a5;
    return request_impl(get_actor_ref(), recver, m);
  }

  void reply(aid_t recver)
  {
    message m;
    reply_impl(get_actor_ref(), recver, m);
  }

  template <typename A1>
  void reply(aid_t recver, A1 const& a1)
  {
    message m;
    m << a1;
    reply_impl(get_actor_ref(), recver, m);
  }

  template <typename A1, typename A2>
  void reply(aid_t recver, A1 const& a1, A2 const& a2)
  {
    message m;
    m << a1 << a2;
    reply_impl(get_actor_ref(), recver, m);
  }

  template <typename A1, typename A2, typename A3>
  void reply(aid_t recver, A1 const& a1, A2 const& a2, A3 const& a3)
  {
    message m;
    m << a1 << a2 << a3;
    reply_impl(get_actor_ref(), recver, m);
  }

  template <typename A1, typename A2, typename A3, typename A4>
  void reply(aid_t recver, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    message m;
    m << a1 << a2 << a3 << a4;
    reply_impl(get_actor_ref(), recver, m);
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void reply(aid_t recver, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    message m;
    m << a1 << a2 << a3 << a4 << a5;
    reply_impl(get_actor_ref(), recver, m);
  }

  /// internal use
  sender()
    : a_(0)
  {
  }

protected:
  inline void set_actor_ref(actor_ref_t& a)
  {
    a_ = &a;
  }

  inline actor_ref_t& get_actor_ref()
  {
    GCE_ASSERT(a_);
    return *a_;
  }

private:
  actor_ref_t* a_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_SENDER_HPP
