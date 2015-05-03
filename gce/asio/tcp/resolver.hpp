///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_TCP_RESOVER_HPP
#define GCE_ASIO_TCP_RESOVER_HPP

#include <gce/asio/config.hpp>
#include <boost/make_shared.hpp>

namespace gce
{
namespace asio
{
namespace tcp
{
static match_t const as_resolve = atom("as_resolve");
/// a wrapper for boost::asio::ip::tcp::resolver
class resolver
  : public addon_t
{
  enum
  {
    ha_resolve = 0,

    ha_num,
  };

  typedef addon_t base_t;
  typedef resolver self_t;
  typedef base_t::scope<self_t, boost::array<gce::detail::handler_allocator_t, ha_num> > scope_t;
  typedef scope_t::guard_ptr guard_ptr;

public:
  typedef boost::asio::ip::tcp::resolver impl_t;

public:
  template <typename Actor>
  explicit resolver(Actor& a)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , impl_(boost::ref(snd_.get_io_service()))
    , resolving_(false)
    , scp_(this)
  {
  }

  ~resolver()
  {
  }

public:
  void async_resolve(impl_t::query const& qry, message const& msg = message(as_resolve))
  {
    GCE_ASSERT(!resolving_);

    resolve_msg_ = msg;
    impl_.async_resolve(
      qry, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment()[ha_resolve],
          boost::bind(
            &self_t::handle_resolve, scp_.get(),
            boost::asio::placeholders::error, boost::asio::placeholders::iterator
            )
          )
        )
      );
    resolving_ = true;
  }

  /// for using other none-async methods directly
  impl_t* operator->()
  {
    return &impl_;
  }

  void dispose()
  {
    scp_.notify();
    impl_.cancel();
  }
  
  impl_t& get_impl()
  {
    return impl_;
  }

private:
  static void handle_resolve(guard_ptr guard, errcode_t const& ec, impl_t::iterator const& itr)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }
    
    o->resolving_ = false;
    message& m = o->resolve_msg_;
    m << ec << boost::make_shared<impl_t::iterator>(itr);
    o->pri_send2actor(m);
  }

private:
  void pri_send2actor(message& m)
  {
    message msg(m);
    m = msg_nil_;
    send2actor(msg);
  }

private:
  strand_t& snd_;
  impl_t impl_;
  bool resolving_;
  
  message resolve_msg_;
  message const msg_nil_;

  /// for quit
  scope_t scp_;
};
}
}
}

#endif /// GCE_ASIO_TCP_RESOVER_HPP
