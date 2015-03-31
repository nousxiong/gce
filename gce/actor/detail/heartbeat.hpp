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
#include <gce/actor/duration.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

namespace gce
{
namespace detail
{
class heartbeat
{
  typedef boost::function<void ()> timeout_func_t;

public:
  explicit heartbeat(strand_t& snd)
    : snd_(snd)
    , tmr_(snd_.get_io_service())
    , sync_(snd_.get_io_service())
    , max_count_(0)
    , curr_count_(0)
    , stopped_(false)
    , waiting_(0)
  {
  }

  ~heartbeat()
  {
  }

public:
  template <typename F>
  void init(
    gce::duration_t period, size_t max_count, 
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

  void start()
  {
    curr_count_ = max_count_;
    stopped_ = false;
    start_timer();
  }

  void stop()
  {
    if (!stopped_)
    {
      stopped_ = true;
      errcode_t ignore_ec;
      tmr_.cancel(ignore_ec);
    }
  }

  void beat()
  {
    if (!stopped_)
    {
      curr_count_ = max_count_;
    }
  }

  void wait_end(yield_t yield)
  {
    if (waiting_ > 0)
    {
      errcode_t ec;
      sync_.expires_from_now(gce::to_chrono(infin));
      sync_.async_wait(yield[ec]);
    }
  }

  void clear()
  {
    timeout_.clear();
    tick_.clear();
  }

private:
  void start_timer()
  {
    ++waiting_;
    tmr_.expires_from_now(gce::to_chrono(period_));
    tmr_.async_wait(
      snd_.wrap(
        boost::bind(
          &heartbeat::handle_timeout, this,
          boost::asio::placeholders::error
          )
        )
      );
  }

  void handle_timeout(errcode_t const& errc)
  {
    --waiting_;
    if (!stopped_ && !errc)
    {
      --curr_count_;
      if (tick_)
      {
        tick_();
      }

      if (curr_count_ == 0)
      {
        timeout_();
      }
      else
      {
        start_timer();
      }
    }

    if (stopped_)
    {
      errcode_t ignore_ec;
      sync_.cancel(ignore_ec);
    }
  }

private:
  strand_t& snd_;
  timer_t tmr_;
  timer_t sync_;
  gce::duration_t period_;
  size_t max_count_;
  size_t curr_count_;

  timeout_func_t timeout_;
  timeout_func_t tick_;
  bool stopped_;
  size_t waiting_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_HEARTBEAT_HPP
