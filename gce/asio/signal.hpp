///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_SIGNAL_HPP
#define GCE_ASIO_SIGNAL_HPP

#include <gce/asio/config.hpp>
#include <boost/bind.hpp>

namespace gce
{
namespace asio
{
static match_t const as_signal = atom("as_signal");

/// a wrapper for boost::asio::signal_set
class signal
  : public addon_t
{
  typedef addon_t base_t;
  typedef signal self_t;
  typedef base_t::scope<self_t, gce::detail::handler_allocator_t> scope_t;
  typedef scope_t::guard_ptr guard_ptr;

public:
  typedef boost::asio::signal_set impl_t;

public:
  template <typename Actor>
  explicit signal(Actor a)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service())
    , waiting_(false)
    , scp_(this)
  {
  }

  template <typename Actor>
  explicit signal(Actor a, int signal_number_1)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service(), signal_number_1)
    , waiting_(false)
    , scp_(this)
  {
  }

  template <typename Actor>
  explicit signal(Actor a, int signal_number_1, int signal_number_2)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service(), signal_number_1, signal_number_2)
    , waiting_(false)
    , scp_(this)
  {
  }

  template <typename Actor>
  explicit signal(Actor a, int signal_number_1, int signal_number_2, int signal_number_3)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(snd_.get_io_service(), signal_number_1, signal_number_2, signal_number_3)
    , waiting_(false)
    , scp_(this)
  {
  }

  ~signal()
  {
  }

public:
  void async_wait(message const& msg = message(as_signal))
  {
    GCE_ASSERT(!waiting_);
    
    wait_msg_ = msg;
    impl_.async_wait(
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment(),
          boost::bind(
            &self_t::handle_wait, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::signal_number
            )
          )
        )
      );
    waiting_ = true;
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
  static void handle_wait(guard_ptr guard, errcode_t const& ec, int signal_number)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->waiting_ = false;
    message& m = o->wait_msg_;
    m << ec << signal_number;

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
}
}

#endif /// GCE_ASIO_SIGNAL_HPP
