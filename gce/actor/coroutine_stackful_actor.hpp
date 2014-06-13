///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_COROUTINE_STACKFUL_ACTOR_HPP
#define GCE_ACTOR_COROUTINE_STACKFUL_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/actor_fwd.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/detail/object_pool.hpp>
#include <gce/actor/detail/mailbox_fwd.hpp>

namespace gce
{
namespace detail
{
class cache_pool;
class mailbox;
}
class thread_mapped_actor;
class context;
class response_t;

template <class> class actor;

class coroutine_stackful_actor
  : public detail::object_pool<coroutine_stackful_actor, detail::cache_pool*>::object
  , public basic_actor
{
  typedef basic_actor base_type;
  enum actor_code
  {
    actor_normal = 0,
    actor_timeout,
  };

public:
  enum status
  {
    ready = 0,
    on,
    off
  };

  inline void send(aid_t recver, message const& m)
  {
    base_type::pri_send(recver, m);
  }

  inline void send(svcid_t recver, message const& m)
  {
    base_type::pri_send_svc(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    base_type::pri_relay(des, m);
  }

  inline void relay(svcid_t des, message& m)
  {
    base_type::pri_relay_svc(des, m);
  }

  inline response_t request(aid_t recver, message const& m)
  {
    response_t res(base_type::new_request(), get_aid(), recver);
    base_type::pri_request(res, recver, m);
    return res;
  }

  inline response_t request(svcid_t recver, message const& m)
  {
    response_t res(base_type::new_request(), get_aid(), recver);
    base_type::pri_request_svc(res, recver, m);
    return res;
  }

  inline void reply(aid_t recver, message const& m)
  {
    base_type::pri_reply(recver, m);
  }

  inline void link(aid_t target)
  {
    base_type::pri_link(target);
  }

  inline void monitor(aid_t target)
  {
    base_type::pri_monitor(target);
  }

  typedef actor<stackful>& self_ref_t;
  typedef boost::function<void (self_ref_t)> func_t;

public:
  explicit coroutine_stackful_actor(detail::cache_pool*);
  ~coroutine_stackful_actor();

public:
  aid_t recv(message&, match const& mach = match());
  aid_t recv(
    response_t, message&, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    );
  void wait(duration_t);

  yield_t get_yield();

public:
  /// internal use
  void start(std::size_t);
  void init(func_t const& f);
  void on_free();
  void on_recv(detail::pack&, base_type::send_hint);

  inline sid_t spawn(detail::spawn_type type, match_t func, match_t ctxid, std::size_t stack_size)
  {
    sid_t sid = base_type::new_request();
    base_type::pri_spawn(sid, type, func, ctxid, stack_size);
    return sid;
  }

private:
  void run(yield_t);
  void resume(actor_code ac = actor_normal);
  actor_code yield();
  void free_self();
  void stop(exit_code_t, std::string);
  void start_recv_timer(duration_t);
  void handle_recv_timeout(errcode_t const&, std::size_t);
  void handle_recv(detail::pack&);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(func_t, f_)

  typedef boost::function<void (actor_code)> yield_cb_t;

  /// thread local vals
  bool recving_;
  bool responsing_;
  detail::recv_t recving_rcv_;
  response_t recving_res_;
  message recving_msg_;
  match curr_match_;
  timer_t tmr_;
  std::size_t tmr_sid_;
  yield_t* yld_;
  yield_cb_t yld_cb_;
  exit_code_t ec_;
  std::string exit_msg_;
};
}

#endif /// GCE_ACTOR_COROUTINE_STACKFUL_ACTOR_HPP
