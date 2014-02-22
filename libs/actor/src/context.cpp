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
#include <gce/detail/cache_aligned_new.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/foreach.hpp>
#include <boost/ref.hpp>
#include <stdexcept>

namespace gce
{
///------------------------------------------------------------------------------
/// context
///------------------------------------------------------------------------------
context::context(attributes attrs)
  : attrs_(attrs)
  , curr_cache_pool_(size_nil)
  , cache_pool_size_(attrs_.thread_num_ * attrs_.per_thread_cache_)
  , ios_(
      attrs_.ios_ ?
        attrs_.ios_ :
        GCE_CACHE_ALIGNED_NEW(io_service_t)
          (attrs_.thread_num_), detail::cache_aligned_deleter()
          )
  , work_(GCE_CACHE_ALIGNED_NEW(io_service_t::work)(*ios_), detail::cache_aligned_deleter())
  , thread_group_(GCE_CACHE_ALIGNED_NEW(boost::thread_group), detail::cache_aligned_deleter())
  , cache_pool_list_(GCE_CACHE_ALIGNED_NEW(cache_pool_list_t), detail::cache_aligned_deleter())
  , mixin_list_(GCE_CACHE_ALIGNED_NEW(mixin_list_t), detail::cache_aligned_deleter())
  , curr_mixin_(0)
{
  cache_pool_list_->reserve(cache_pool_size_);
  mixin_list_->reserve(attrs_.mixin_num_);

  try
  {
    std::size_t index = 0;
    for (std::size_t i=0; i<cache_pool_size_; ++i, ++index)
    {
      cache_pool_list_->push_back(cache_pool_ptr());
      cache_pool_ptr& cac_pool = cache_pool_list_->back();
      cac_pool = cache_pool_ptr(GCE_CACHE_ALIGNED_NEW(detail::cache_pool)(*this, index, attrs_, false));
      start_gc_timer(cac_pool.get());
    }

    for (std::size_t i=0; i<attrs_.thread_num_; ++i)
    {
      thread_group_->create_thread(
        boost::bind(
          &context::run, this, i,
          attrs_.thread_begin_cb_list_,
          attrs_.thread_end_cb_list_
          )
        );
    }

    for (std::size_t i=0; i<attrs_.mixin_num_; ++i, index+=attrs_.per_mixin_cache_)
    {
      mixin_list_->push_back(mixin_ptr());
      mixin_ptr& mi = mixin_list_->back();
      mi = mixin_ptr(GCE_CACHE_ALIGNED_NEW(mixin)(*this, index, attrs_));
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
detail::cache_pool* context::select_cache_pool()
{
  std::size_t curr_cache_pool = curr_cache_pool_;
  ++curr_cache_pool;
  if (curr_cache_pool >= cache_pool_size_)
  {
    curr_cache_pool = 0;
  }
  curr_cache_pool_ = curr_cache_pool;
  return (*cache_pool_list_)[curr_cache_pool].get();
}
///------------------------------------------------------------------------------
mixin& context::make_mixin()
{
  std::size_t i = curr_mixin_.fetch_add(1, boost::memory_order_relaxed);
  if (i >= mixin_list_->size())
  {
    throw std::runtime_error("out of mixin fix num");
  }
  return *(*mixin_list_)[i];
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
  work_.reset();
  BOOST_FOREACH(cache_pool_ptr& cac_pool, *cache_pool_list_)
  {
    if (cac_pool)
    {
      errcode_t ignored_ec;
      cac_pool->get_gc_timer()->cancel(ignored_ec);
    }
  }
  thread_group_->join_all();

  BOOST_FOREACH(cache_pool_ptr& cac_pool, *cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->free_cache();
    }
  }

  BOOST_FOREACH(mixin_ptr& mi, *mixin_list_)
  {
    if (mi)
    {
      mi->free_cache();
    }
  }

  BOOST_FOREACH(mixin_ptr& mi, *mixin_list_)
  {
    GCE_CACHE_ALIGNED_DELETE(mixin, mi.get());
  }

  BOOST_FOREACH(cache_pool_ptr& cac_pool, *cache_pool_list_)
  {
    GCE_CACHE_ALIGNED_DELETE(detail::cache_pool, cac_pool.get());
  }
}
///------------------------------------------------------------------------------
void context::start_gc_timer(detail::cache_pool* cac_pool)
{
  timer_t* gc_tmr = cac_pool->get_gc_timer();
  strand_t* snd = cac_pool->get_strand();
  gc_tmr->expires_from_now(attrs_.gc_period_);
  gc_tmr->async_wait(
    snd->wrap(
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
