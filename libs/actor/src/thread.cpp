///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/thread.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/context.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <stdexcept>

namespace gce
{
///------------------------------------------------------------------------------
/// thread
///------------------------------------------------------------------------------
thread::thread(context& ctx, thrid_t thrid)
  : ctx_(&ctx)
  , ctxid_(ctx.get_attributes().id_)
  , timestamp_(ctx.get_timestamp())
  , max_cache_match_size_(ctx.get_attributes().max_cache_match_size_)
  , thrid_(thrid)
  , ios_(1)
  , cac_pool_(*this, ctx.get_attributes())
  , stopped_(false)
  , max_wait_counter_size_(ctx.get_attributes().max_wait_counter_size_)
{
}
///------------------------------------------------------------------------------
thread::~thread()
{
}
///------------------------------------------------------------------------------
void release_pack(detail::pack* pk)
{
  if (pk->thr_)
  {
    pk->thr_->get_cache_pool().free_pack(pk);
  }
  else if (pk->que_)
  {
    pk->que_->push(pk);
  }
}
///------------------------------------------------------------------------------
void thread::run(
  std::vector<thread_callback_t> const& begin_cb_list,
  std::vector<thread_callback_t> const& end_cb_list
  )
{
  BOOST_FOREACH(thread_callback_t const& cb, begin_cb_list)
  {
    cb(thrid_);
  }

  ctxid_t ctxid = ctxid_;
  timestamp_t timestamp = timestamp_;
  int counter = max_wait_counter_size_;
  int yield_count = counter / 10;

  while (!stopped_)
  {
    detail::pack* pk = pack_queue_.pop_all();
    if (pk)
    {
      do
      {
        try
        {
          detail::pack* next = detail::node_access::get_next(pk);
          detail::node_access::set_next(pk, (detail::pack*)0);
          detail::scope scp(boost::bind(&release_pack, pk));
          detail::pack* curr = pk;
          pk = next;

          if (curr->target_)
          {
            curr->target_.get_actor_ptr(ctxid, timestamp)->on_recv(curr);
          }
          else
          {
            BOOST_ASSERT(curr->f_);
            curr->f_();
          }
        }
        catch (...)
        {
          std::printf("Unexpected exception from pack: %s\n",
            boost::current_exception_diagnostic_information().c_str());
        }
      } 
      while (pk);
      counter = max_wait_counter_size_;
    }
    else
    {
      if (counter > yield_count)
      {
        --counter;
      }
      else if (counter > 0)
      {
        --counter;
        boost::this_thread::yield();
      }
      else
      {
        boost::this_thread::sleep_for(boost::chrono::milliseconds(1));
      }
    }

    try
    {
      ios_.poll();
      cac_pool_.free_object();
    }
    catch (...)
    {
      std::printf("Unexpected exception from io_service: %s\n",
          boost::current_exception_diagnostic_information().c_str());
    }
  }

  BOOST_FOREACH(thread_callback_t const& cb, end_cb_list)
  {
    cb(thrid_);
  }
}
///------------------------------------------------------------------------------
void thread::stop()
{
  cac_pool_.stop();
  stopped_ = true;
}
///------------------------------------------------------------------------------
}
