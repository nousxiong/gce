///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/detail/heartbeat.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>

namespace gce
{
namespace detail
{
///----------------------------------------------------------------------------
heartbeat::heartbeat(io_service_t& ios)
  : tmr_(ios)
  , sync_(ios)
  , max_count_(0)
  , curr_count_(0)
  , stopped_(false)
  , waiting_(0)
{
}
///----------------------------------------------------------------------------
heartbeat::~heartbeat()
{
}
///----------------------------------------------------------------------------
void heartbeat::start()
{
  curr_count_ = max_count_;
  stopped_ = false;
  start_timer();
}
///----------------------------------------------------------------------------
void heartbeat::stop()
{
  if (!stopped_)
  {
    stopped_ = true;
    errcode_t ignore_ec;
    tmr_.cancel(ignore_ec);
  }
}
///----------------------------------------------------------------------------
void heartbeat::beat()
{
  if (!stopped_)
  {
    curr_count_ = max_count_;
  }
}
///----------------------------------------------------------------------------
void heartbeat::wait_end(yield_t yield)
{
  if (waiting_ > 0)
  {
    errcode_t ec;
    sync_.expires_from_now(infin);
    sync_.async_wait(yield[ec]);
  }
}
///----------------------------------------------------------------------------
void heartbeat::clear()
{
  timeout_.clear();
  tick_.clear();
}
///----------------------------------------------------------------------------
void heartbeat::start_timer()
{
  ++waiting_;
  tmr_.expires_from_now(period_);
  tmr_.async_wait(
    boost::bind(
      &heartbeat::handle_timeout, this,
      boost::asio::placeholders::error
      )
    );
}
///----------------------------------------------------------------------------
void heartbeat::handle_timeout(errcode_t const& errc)
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
///----------------------------------------------------------------------------
}
}
