///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_DETAIL_BASIC_TIMER_HPP
#define GCE_ASIO_DETAIL_BASIC_TIMER_HPP

#include <gce/asio/config.hpp>
#include <boost/bind.hpp>

namespace gce
{
namespace asio
{
static match_t const as_timeout = atom("as_timeout");
namespace detail
{
/// a wrapper for boost::asio::xxx_timer
template <typename Impl, int ty>
class basic_timer
  : public addon_t
{
  typedef addon_t base_t;
  typedef basic_timer<Impl, ty> self_t;
  typedef base_t::scope<self_t, gce::detail::handler_allocator_t> scope_t;
  typedef typename scope_t::guard_ptr guard_ptr;

public:
  typedef Impl impl_t;
  static int type()
  {
    return ty;
  }

public:
  template <typename Actor>
  explicit basic_timer(Actor a)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service())
    , waiting_(false)
    , scp_(this)
  {
  }

  ~basic_timer()
  {
  }

public:
  void async_wait(duration_t dur, message const& msg = message(as_timeout))
  {
    GCE_ASSERT(!waiting_);
    impl_.expires_from_now(to_chrono(dur));
    pri_async_wait(msg);
  }

  
  void async_wait(message const& msg = message(as_timeout))
  {
    GCE_ASSERT(!waiting_);
    pri_async_wait(msg);
  }

  /// for using other none-async methods directly
  impl_t* operator->()
  {
    return &impl_;
  }

  void dispose()
  {
    scp_.notify();
    errcode_t ignored_ec;
    impl_.cancel(ignored_ec);
  }

private:
  void pri_async_wait(message const& msg)
  {
    wait_msg_ = msg;
    impl_.async_wait(
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment(),
          boost::bind(
            &self_t::handle_wait, scp_.get(),
            boost::asio::placeholders::error
            )
          )
        )
      );
    waiting_ = true;
  }

  static void handle_wait(guard_ptr guard, errcode_t const& ec)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->waiting_ = false;
    message& m = o->wait_msg_;
    m << ec;

    message msg(m);
    m = o->msg_nil_;
    o->send2actor(msg);
  }

private:
  strand_t& snd_;
  impl_t impl_;
  bool waiting_;

  message wait_msg_;
  message const msg_nil_;

  /// for quit
  scope_t scp_;
};
} /// namespace detail
} /// namespace asio
} /// namespace gce

#endif /// GCE_ASIO_DETAIL_BASIC_TIMER_HPP
