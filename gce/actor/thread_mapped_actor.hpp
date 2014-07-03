///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_MIXIN_HPP
#define GCE_ACTOR_MIXIN_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <boost/thread/future.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace gce
{
class thread_mapped_actor;
class nonblocking_actor;
class context;
struct attributes;
namespace detail
{
class cache_pool;
}

class thread_mapped_actor
  : public basic_actor
{
  typedef basic_actor base_type;

public:
  explicit thread_mapped_actor(detail::cache_pool*);
  ~thread_mapped_actor();

public:
  void send(aid_t, message const&);
  void send(svcid_t, message const&);
  void relay(aid_t, message&);
  void relay(svcid_t, message&);

  response_t request(aid_t, message const&);
  response_t request(svcid_t, message const&);
  void reply(aid_t, message const&);

  void link(aid_t);
  void monitor(aid_t);

  aid_t recv(message&, match const& mach = match());
  aid_t recv(
    response_t, message&,
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    );
  void wait(duration_t);

public:
  /// internal use
  inline void add_nonblocking_actor(nonblocking_actor& a)
  {
    nonblocking_actor_list_.push_back(&a);
  }
  inline std::vector<nonblocking_actor*>& get_nonblocking_actor_list() 
  { 
    return nonblocking_actor_list_; 
  }
  void on_recv(detail::pack&, base_type::send_hint);

  sid_t spawn(detail::spawn_type, match_t func, match_t ctxid, std::size_t stack_size);

private:
  typedef boost::optional<std::pair<detail::recv_t, message> > recv_optional_t;
  typedef boost::optional<std::pair<response_t, message> > res_optional_t;
  typedef boost::promise<recv_optional_t> recv_promise_t;
  typedef boost::promise<res_optional_t> res_promise_t;
  typedef boost::unique_future<recv_optional_t> recv_future_t;
  typedef boost::unique_future<res_optional_t> res_future_t;
  void try_recv(recv_promise_t&, match const&);
  void try_response(res_promise_t&, response_t, duration_t);
  void start_recv_timer(duration_t, recv_promise_t&);
  void start_recv_timer(duration_t, res_promise_t&);
  void handle_recv_timeout(errcode_t const&, recv_promise_t&, std::size_t);
  void handle_res_timeout(errcode_t const&, res_promise_t&, std::size_t);

  void handle_recv(detail::pack&);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  /// local
  std::vector<nonblocking_actor*> nonblocking_actor_list_;
  recv_promise_t* recv_p_;
  res_promise_t* res_p_;
  response_t recving_res_;
  match curr_match_;
  timer_t tmr_;
  std::size_t tmr_sid_;
};
}

#endif /// GCE_ACTOR_MIXIN_HPP
