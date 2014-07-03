///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_HEARTBEAT_HPP
#define GCE_ACTOR_DETAIL_HEARTBEAT_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <boost/function.hpp>

namespace gce
{
namespace detail
{
class cache_pool;
class heartbeat
{
  typedef boost::function<void ()> timeout_func_t;

public:
  explicit heartbeat(strand_t&);
  ~heartbeat();

public:
  template <typename F>
  void init(
    seconds_t period, std::size_t max_count, 
    F f, F t = timeout_func_t()
    )
  {
    clear();
    period_ = period;
    max_count_ = max_count;
    if (max_count_ == 0)
    {
      max_count_ = 1;
    }
    timeout_ = f;
    tick_ = t;
  }

  void start();
  void stop();
  void beat();
  void wait_end(yield_t);

  void clear();

private:
  void start_timer();
  void handle_timeout(errcode_t const&);

private:
  strand_t& snd_;
  timer_t tmr_;
  timer_t sync_;
  seconds_t period_;
  std::size_t max_count_;
  std::size_t curr_count_;

  timeout_func_t timeout_;
  timeout_func_t tick_;
  bool stopped_;
  std::size_t waiting_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_HEARTBEAT_HPP
