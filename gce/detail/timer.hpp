///
/// timer.hpp
///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_TIMER_HPP
#define GCE_DETAIL_TIMER_HPP

#include <gce/config.hpp>
#include <gce/integer.hpp>
#include <gce/detail/asio_alloc_handler.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/function.hpp>

namespace gce
{
namespace detail
{
class timer
{
  typedef timer self_t;
  typedef boost::function<void (boost::system::error_code const&)> handler_t;
  
public:
  explicit timer(boost::asio::io_service::strand const& snd)
    : snd_(snd)
    , tmr_(snd_.get_io_service())
    , snid_(0)
  {
  }
  
public:
  template <typename Handler>
  void async_wait(boost::asio::system_timer::duration dur, Handler const& h)
  {
    ec_.clear();
    tmr_.expires_from_now(dur);
    tmr_.async_wait(
      snd_.wrap(
        make_asio_alloc_handler(
          ha_, 
          wait_binder(*this, h, snid_)
          )
        )
      );
  }
  
  void cancel()
  {
    ++snid_;
    boost::system::error_code ignored_ec;
    tmr_.cancel(ignored_ec);
  }
  
  bool timed_out() const
  {
    return ec_ == boost::asio::error::timed_out;
  }
  
private:
  bool cancelled(uint64_t snid) const
  {
    return snid_ != snid;
  }
  
  struct wait_binder
  {
    template <typename Handler>
    wait_binder(self_t& self, Handler const& h, uint64_t snid)
      : self_(self)
      , h_(h)
      , snid_(snid)
    {
    }
    
    void operator()(boost::system::error_code const& ec) const
    {
      self_t::handle_timeout(self_, ec, h_, snid_);
    }
    
    self_t& self_;
    handler_t h_;
    uint64_t snid_;
  };
  
  static void handle_timeout(
    self_t& self, 
    boost::system::error_code const& ec, 
    handler_t const& h,
    uint64_t snid
    )
  {
    if (!self.cancelled(snid) && ec != boost::asio::error::operation_aborted)
    {
      self.ec_ = boost::asio::error::timed_out;
    }
    
    h(self.ec_);
  }
  
private:
  boost::asio::io_service::strand snd_;
  boost::asio::system_timer tmr_;
  boost::system::error_code ec_;
  uint64_t snid_;
  handler_allocator_t ha_;
};
}
}

#endif /// GCE_DETAIL_TIMER_HPP
