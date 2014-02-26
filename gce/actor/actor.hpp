///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ACTOR_HPP
#define GCE_ACTOR_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/actor_fwd.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/detail/mailbox_fwd.hpp>

namespace gce
{
namespace detail
{
class cache_pool;
class mailbox;
}
class mixin;
class response_t;

class actor
  : public detail::object_pool<actor, detail::actor_attrs>::object
  , public detail::mpsc_queue<actor>::node
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

  class self
  {
  public:
    explicit self(actor& a) : a_(a) {}
    ~self() {}

  public:
    inline aid_t recv(message& msg, match const& mach = match())
    {
      return a_.recv(msg, mach);
    }

    inline void send(aid_t recver, message const& m)
    {
      a_.send(recver, m);
    }

    inline void relay(aid_t des, message& m)
    {
      a_.relay(des, m);
    }

    inline response_t request(aid_t recver, message const& m)
    {
      return a_.request(recver, m);
    }

    inline void reply(aid_t recver, message const& m)
    {
      a_.reply(recver, m);
    }

    inline void wait(duration_t dur)
    {
      a_.wait(dur);
    }

    inline aid_t recv(response_t res, message& msg, duration_t tmo = infin)
    {
      return a_.recv(res, msg, tmo);
    }

    inline void link(aid_t target)
    {
      a_.link(target);
    }

    inline void monitor(aid_t target)
    {
      a_.monitor(target);
    }

    inline aid_t get_aid()
    {
      return a_.get_aid();
    }

    inline detail::cache_pool* get_cache_pool()
    {
      return a_.get_cache_pool();
    }

    inline yield_t get_yield()
    {
      return a_.get_yield();
    }

    /// internal use
    inline void add_link(detail::link_t l)
    {
      a_.add_link(l);
    }

  private:
    actor& a_;
  };

  typedef self& self_ref_t;
  typedef boost::function<void (self_ref_t)> func_t;

public:
  explicit actor(detail::actor_attrs);
  ~actor();

public:
  aid_t recv(message&, match const&);
  void send(aid_t, message const&);
  void relay(aid_t, message&);

  response_t request(aid_t, message const&);
  void reply(aid_t, message const&);
  aid_t recv(response_t, message&, duration_t);
  void wait(duration_t);

  void link(aid_t);
  void monitor(aid_t);

public:
  detail::cache_pool* get_cache_pool();
  yield_t get_yield();
  void start(std::size_t);
  void init(
    detail::cache_pool* user, detail::cache_pool* owner,
    func_t const& f,
    aid_t link_tgt
    );
  void on_free();
  void on_recv(detail::pack*);

private:
  void run(yield_t);
  void resume(actor_code ac = actor_normal);
  actor_code yield();
  void free_self();
  void stop(exit_code_t, std::string const&);
  void start_recv_timer(duration_t);
  void handle_recv_timeout(errcode_t const&, std::size_t);
  void handle_recv(detail::pack*);

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(self, self_)
  GCE_CACHE_ALIGNED_VAR(detail::cache_pool*, user_)
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

typedef actor::self& self_t;
typedef boost::function<void (self_t)> actor_func_t;
}

#endif /// GCE_ACTOR_ACTOR_HPP
