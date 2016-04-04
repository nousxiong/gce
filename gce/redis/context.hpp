///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_CONTEXT_HPP
#define GCE_REDIS_CONTEXT_HPP

#include <gce/redis/config.hpp>
#include <gce/redis/session.hpp>
#include <gce/redis/context_id.hpp>
#include <gce/redis/conn.hpp>
#include <gce/redis/detail/conn_impl.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

namespace gce
{
namespace redis
{
class context
{
  typedef detail::conn_impl<session> conn_impl_t;

public:
  explicit context(gce::context& ctx)
    : ctxid_(make_ctxid(this))
    , stopped_(false)
    , base_(gce::spawn(ctx))
    , ios_(ctx.get_io_service())
    , conn_list_(ctx.get_concurrency_size())
    , lg_(ctx.get_logger())
  {
  }

  ~context()
  {
    stop();
  }

public:
  ctxid_t get_ctxid() const
  {
    return ctxid_;
  }

  conn_t make_conn(std::string const& host, uint16_t port, gce::duration_t timeout)
  {
    GCE_ASSERT(!stopped_);
    conn_t c;
    conn_impl_t* impl = new conn_impl_t(ios_, host, port, timeout);
    conn_list_.push(impl);
    c.ptr_ = (uint64_t)impl;
    return c;
  }

  conn_t make_conn(boost::asio::ip::tcp::resolver::iterator eitr, gce::duration_t timeout)
  {
    GCE_ASSERT(!stopped_);
    conn_t c;
    conn_impl_t* impl = new conn_impl_t(ios_, eitr, timeout);
    conn_list_.push(impl);
    c.ptr_ = (uint64_t)impl;
    return c;
  }

private:
  void stop()
  {
    /// close all conns
    stopped_ = true;
    conn_impl_t* ci = 0;
    while (conn_list_.pop(ci))
    {
      ci->snd_.post(dispose_binder(ci));
    }
  }

  struct dispose_binder
  {
    explicit dispose_binder(conn_impl_t* ci)
      : ci_(ci)
    {
    }

    void operator()() const
    {
      if (ci_->disposable())
      {
        delete ci_;
      }
      else
      {
        ci_->snd_.post(dispose_binder(ci_));
      }
    }

    conn_impl_t* ci_;
  };

private:
  ctxid_t ctxid_;
  bool stopped_;
  gce::threaded_actor base_;

  io_service_t& ios_;

  boost::lockfree::queue<conn_impl_t*> conn_list_;
  log::logger_t lg_;
};

/// Make a conn with host and port. 
/// timeout is connect and query actions max time.
static conn_t make_conn(
  ctxid_t const& ctxid, 
  std::string const& host, int16_t port, 
  gce::duration_t timeout = gce::seconds(3)
  )
{
  context* ctx = get_context(ctxid);
  return ctx->make_conn(host, port, timeout);
}

/// Make a conn with resolver::iterator. 
/// timeout is connect and query actions max time.
static conn_t make_conn(
  ctxid_t const& ctxid, 
  boost::asio::ip::tcp::resolver::iterator eitr, 
  gce::duration_t timeout = gce::seconds(15)
  )
{
  context* ctx = get_context(ctxid);
  return ctx->make_conn(eitr, timeout);
}
}
}

#endif /// GCE_REDIS_CONTEXT_HPP
