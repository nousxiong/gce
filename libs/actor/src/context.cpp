///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/mixin.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <stdexcept>
#include <ctime>

namespace gce
{
///------------------------------------------------------------------------------
/// context
///------------------------------------------------------------------------------
context::context(attributes attrs)
  : attrs_(attrs)
  , timestamp_((timestamp_t)boost::chrono::system_clock::now().time_since_epoch().count())
  , curr_cache_pool_(size_nil)
  , cache_pool_size_(attrs_.thread_num_ * attrs_.per_thread_cache_)
{
  if (attrs_.ios_)
  {
    ios_.reset(attrs_.ios_, detail::empty_deleter<io_service_t>());
  }
  else
  {
    ios_.reset(new io_service_t(attrs_.thread_num_));
  }
  work_ = boost::in_place(boost::ref(*ios_));
  cache_pool_list_.reserve(cache_pool_size_);

  try
  {
    std::size_t index = 0;
    for (std::size_t i=0; i<cache_pool_size_; ++i, ++index)
    {
      cache_pool_list_.push_back((detail::cache_pool*)0);
      detail::cache_pool*& cac_pool = cache_pool_list_.back();
      cac_pool = new detail::cache_pool(*this, index, attrs_, false);
      start_gc_timer(cac_pool);
    }

    for (std::size_t i=0; i<attrs_.thread_num_; ++i)
    {
      thread_group_.create_thread(
        boost::bind(
          &context::run, this, i,
          attrs_.thread_begin_cb_list_,
          attrs_.thread_end_cb_list_
          )
        );
    }
  }
  catch (...)
  {
    stop();
    throw;
  }
}
///------------------------------------------------------------------------------
context::~context()
{
  stop();
}
///------------------------------------------------------------------------------
mixin& context::make_mixin()
{
  mixin* mix = new mixin(*this, attrs_);
  mixin_list_.push(mix);
  return *mix;
}
///------------------------------------------------------------------------------
detail::cache_pool* context::select_cache_pool()
{
  std::size_t curr_cache_pool = curr_cache_pool_;
  ++curr_cache_pool;
  if (curr_cache_pool >= cache_pool_size_)
  {
    curr_cache_pool = 0;
  }
  curr_cache_pool_ = curr_cache_pool;
  return cache_pool_list_[curr_cache_pool];
}
///------------------------------------------------------------------------------
void context::register_service(match_t name, aid_t svc)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->get_strand().dispatch(
        boost::bind(
          &detail::cache_pool::register_service,
          cac_pool, name, svc
          )
        );
    }
  }
}
///------------------------------------------------------------------------------
void context::deregister_service(match_t name, aid_t svc)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->get_strand().dispatch(
        boost::bind(
          &detail::cache_pool::deregister_service,
          cac_pool, name, svc
          )
        );
    }
  }
}
///------------------------------------------------------------------------------
void context::register_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->get_strand().dispatch(
        boost::bind(
          &detail::cache_pool::register_socket,
          cac_pool, ctxid_pr, skt
          )
        );
    }
  }
}
///------------------------------------------------------------------------------
void context::deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->get_strand().dispatch(
        boost::bind(
          &detail::cache_pool::deregister_socket,
          cac_pool, ctxid_pr, skt
          )
        );
    }
  }
}
///------------------------------------------------------------------------------
void context::run(
  thrid_t id,
  std::vector<thread_callback_t> const& begin_cb_list,
  std::vector<thread_callback_t> const& end_cb_list
  )
{
  BOOST_FOREACH(thread_callback_t const& cb, begin_cb_list)
  {
    cb(id);
  }

  while (true)
  {
    try
    {
      ios_->run();
      break;
    }
    catch (...)
    {
      std::cerr << "Unexpected exception: " <<
        boost::current_exception_diagnostic_information();
    }
  }

  BOOST_FOREACH(thread_callback_t const& cb, end_cb_list)
  {
    cb(id);
  }
}
///------------------------------------------------------------------------------
void context::stop()
{
  work_ = boost::none;
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->get_strand().dispatch(
        boost::bind(&detail::cache_pool::stop, cac_pool)
        );
    }
  }

  thread_group_.join_all();

  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->free_cache();
    }
  }

  mixin* mix = mixin_list_.pop_all_reverse();
  while (mix)
  {
    mixin* next = detail::node_access::get_next(mix);
    detail::node_access::set_next(mix, (mixin*)0);
    delete mix;
    mix = next;
  }

  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    delete cac_pool;
  }
}
///------------------------------------------------------------------------------
void context::start_gc_timer(detail::cache_pool* cac_pool)
{
  timer_t& gc_tmr = cac_pool->get_gc_timer();
  strand_t& snd = cac_pool->get_strand();
  gc_tmr.expires_from_now(attrs_.gc_period_);
  gc_tmr.async_wait(
    snd.wrap(
      boost::bind(
        &context::gc, this,
        cac_pool, boost::asio::placeholders::error
        )
      )
    );
}
///------------------------------------------------------------------------------
void context::gc(detail::cache_pool* cac_pool, errcode_t const& errc)
{
  if (errc != boost::asio::error::operation_aborted)
  {
    start_gc_timer(cac_pool);
  }

  /// free all cache
  cac_pool->free_cache();

  /// do gc
  cac_pool->free_object();
}
///------------------------------------------------------------------------------
}
