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
  , timestamp_((timestamp_t)std::time(NULL))
  , curr_cache_pool_(size_nil)
  , cache_pool_size_(attrs_.thread_num_ * attrs_.per_thread_cache_)
  , curr_mixin_(0)
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
  mixin_list_.reserve(attrs_.mixin_num_);

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

    for (std::size_t i=0; i<attrs_.mixin_num_; ++i, ++index)
    {
      mixin_list_.push_back((mixin*)0);
      mixin*& mi = mixin_list_.back();
      mi = new mixin(*this, index, attrs_);
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
  std::size_t i = curr_mixin_.fetch_add(1, boost::memory_order_relaxed);
  if (i >= mixin_list_.size())
  {
    throw std::runtime_error("out of mixin fix num");
  }
  return *(mixin_list_[i]);
}
///------------------------------------------------------------------------------
detail::cache_pool* context::select_cache_pool(std::size_t i)
{
  if (i == size_nil)
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
  else
  {
    BOOST_ASSERT(i < cache_pool_list_.size());
    return cache_pool_list_[i];
  }
}
///------------------------------------------------------------------------------
void context::register_socket(ctxid_t ctxid, aid_t skt, detail::cache_pool* user)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->get_strand().dispatch(
        boost::bind(
          &detail::cache_pool::register_socket,
          cac_pool, ctxid, skt
          )
        );
    }
  }

  BOOST_FOREACH(mixin* mi, mixin_list_)
  {
    if (mi)
    {
      mi->register_socket(ctxid, skt, user);
    }
  }
}
///------------------------------------------------------------------------------
void context::deregister_socket(ctxid_t ctxid, aid_t skt, detail::cache_pool* user)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->get_strand().dispatch(
        boost::bind(
          &detail::cache_pool::deregister_socket,
          cac_pool, ctxid, skt
          )
        );
    }
  }

  BOOST_FOREACH(mixin* mi, mixin_list_)
  {
    if (mi)
    {
      mi->deregister_socket(ctxid, skt, user);
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
        boost::bind(&context::stop_mixin, this, cac_pool)
        );
      break;
    }
  }

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

  BOOST_FOREACH(mixin* mi, mixin_list_)
  {
    if (mi)
    {
      mi->free_cache();
    }
  }

  BOOST_FOREACH(mixin* mi, mixin_list_)
  {
    delete mi;
  }

  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    delete cac_pool;
  }
}
///------------------------------------------------------------------------------
void context::stop_mixin(detail::cache_pool* user)
{
  BOOST_FOREACH(mixin* mi, mixin_list_)
  {
    if (mi)
    {
      mi->stop(user);
    }
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
