#include <gce/actor/detail/heartbeat.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>

namespace gce
{
namespace detail
{
///----------------------------------------------------------------------------
heartbeat::heartbeat(io_service_t& ios)
  : cac_pool_(0)
  , tmr_(ios)
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
  if (tick_)
  {
    tick_();
  }
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
  cac_pool_ = 0;
  timeout_.clear();
  tick_.clear();
}
///----------------------------------------------------------------------------
void heartbeat::start_timer()
{
  ++waiting_;
  strand_t* snd = cac_pool_->get_strand();
  tmr_.expires_from_now(period_);
  tmr_.async_wait(
    snd->wrap(
      boost::bind(
        &heartbeat::handle_timeout, this,
        boost::asio::placeholders::error
        )
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
