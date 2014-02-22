///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_THIN_HPP
#define GCE_ACTOR_THIN_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/object_pool.hpp>
#include <gce/detail/mpsc_queue.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/detail/mailbox_fwd.hpp>
#include <gce/detail/cache_aligned_new.hpp>

namespace gce
{
namespace detail
{
class cache_pool;
}
class thin
  : public detail::object_pool<thin, detail::thin_attrs>::object
  , public detail::mpsc_queue<thin>::node
  , public basic_actor
{
  typedef basic_actor base_type;

public:
  enum status
  {
    ready = 0,
    on,
    off,
    closed
  };

  typedef boost::function<void (thin&)> func_t;

public:
  explicit thin(detail::thin_attrs);
  ~thin();

public:
  void recv(aid_t&, message&, match const& mach = match());
  aid_t recv(message&, match_list_t const& match_list = match_list_t());
  void send(aid_t, message const&);

  response_t request(aid_t, message const&);
  void reply(aid_t, message const&);
  void recv(response_t, aid_t&, message&, duration_t tmo = infin);
  aid_t recv(response_t, message&);
  void wait(duration_t);

  void link(aid_t);
  void monitor(aid_t);

  inline detail::coro_t& coro() { return coro_; }

public:
  detail::cache_pool* get_cache_pool();
  void init(
    detail::cache_pool* user,
    detail::cache_pool* owner,
    func_t const& f, aid_t
    );
  void start();
  void on_free();
  void on_recv(detail::pack*);

private:
  void run();
  void handle_recv(detail::pack*);
  void begin_run();
  void free_self(exit_code_t, std::string const&);
  void start_recv_timer(duration_t);
  void handle_recv_timeout(errcode_t const&, std::size_t);
  void end_recv();
  void end_response();
  void end_wait();

private:
  detail::cache_aligned_ptr<detail::cache_pool, detail::cache_pool*> user_;

  typedef detail::unique_ptr<func_t> func_ptr;
  detail::cache_aligned_ptr<func_t, func_ptr> f_;

  detail::coro_t coro_;
  byte_t pad1_[GCE_CACHE_LINE_SIZE - sizeof(detail::coro_t)];

  /// thread local vals
  bool recving_;
  bool responsing_;
  detail::recv_t rcv_;
  aid_t* sender_;
  message* msg_;
  response_t res_;
  match curr_mach_;

  timer_t tmr_;
  std::size_t tmr_sid_;
};

typedef thin& thin_t;
typedef boost::function<void (thin_t)> thin_func_t;
}

#endif /// GCE_ACTOR_THIN_HPP

