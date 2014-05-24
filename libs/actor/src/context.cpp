///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/context.hpp>
#include <gce/actor/nonblocking_actor.hpp>
#include <gce/actor/detail/cache_pool.hpp>
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
  , cache_pool_size_(
      attrs_.thread_num_ == 0 ? 
        attrs_.per_thread_cache_pool_num_ : 
        attrs_.thread_num_ * attrs_.per_thread_cache_pool_num_
      )
  , cache_queue_size_(cache_pool_size_ + attrs_.slice_num_)
  , curr_nonblocking_actor_(0)
  , thread_mapped_actor_list_(cache_pool_size_)
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
  cache_pool_list_.resize(cache_pool_size_, 0);
  nonblocking_actor_list_.resize(attrs_.slice_num_, 0);

  try
  {
    std::size_t index = 0;
    for (std::size_t i=0; i<cache_pool_size_; ++i, ++index)
    {
      cache_pool_list_[i] = new detail::cache_pool(*this, index);
    }

    for (std::size_t i=0; i<attrs_.slice_num_; ++i, ++index)
    {
      nonblocking_actor_list_[i] = new nonblocking_actor(*this, index);
    }
    
    base_ = boost::in_place(select_cache_pool());

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
thread_mapped_actor& context::make_thread_mapped_actor()
{
  thread_mapped_actor* a = new thread_mapped_actor(select_cache_pool());
  thread_mapped_actor_list_.push(a);
  return *a;
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
nonblocking_actor& context::make_nonblocking_actor()
{
  std::size_t i = curr_nonblocking_actor_.fetch_add(1, boost::memory_order_relaxed);
  if (i >= nonblocking_actor_list_.size())
  {
    throw std::runtime_error("out of nonblocking_actor fix size");
  }
  return *(nonblocking_actor_list_[i]);
}
///------------------------------------------------------------------------------
void context::register_service(match_t name, aid_t svc, std::size_t cache_queue_index)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    cac_pool->get_strand().dispatch(
      boost::bind(
        &detail::cache_pool::register_service,
        cac_pool, name, svc
        )
      );
  }

  BOOST_FOREACH(nonblocking_actor* s, nonblocking_actor_list_)
  {
    s->register_service(name, svc, cache_queue_index);
  }
}
///------------------------------------------------------------------------------
void context::deregister_service(match_t name, aid_t svc, std::size_t cache_queue_index)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    cac_pool->get_strand().dispatch(
      boost::bind(
        &detail::cache_pool::deregister_service,
        cac_pool, name, svc
        )
      );
  }

  BOOST_FOREACH(nonblocking_actor* s, nonblocking_actor_list_)
  {
    s->deregister_service(name, svc, cache_queue_index);
  }
}
///------------------------------------------------------------------------------
void context::register_socket(ctxid_pair_t ctxid_pr, aid_t skt, std::size_t cache_queue_index)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    cac_pool->get_strand().dispatch(
      boost::bind(
        &detail::cache_pool::register_socket,
        cac_pool, ctxid_pr, skt
        )
      );
  }

  BOOST_FOREACH(nonblocking_actor* s, nonblocking_actor_list_)
  {
    s->register_socket(ctxid_pr, skt, cache_queue_index);
  }
}
///------------------------------------------------------------------------------
void context::deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt, std::size_t cache_queue_index)
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    cac_pool->get_strand().dispatch(
      boost::bind(
        &detail::cache_pool::deregister_socket,
        cac_pool, ctxid_pr, skt
        )
      );
  }

  BOOST_FOREACH(nonblocking_actor* s, nonblocking_actor_list_)
  {
    s->deregister_socket(ctxid_pr, skt, cache_queue_index);
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
    cac_pool->get_strand().dispatch(
      boost::bind(&detail::cache_pool::stop, cac_pool)
      );
  }

  thread_group_.join_all();

  base_.reset();

  thread_mapped_actor* mix = 0;
  while (thread_mapped_actor_list_.pop(mix))
  {
    delete mix;
  }

  BOOST_FOREACH(nonblocking_actor* s, nonblocking_actor_list_)
  {
    delete s;
  }

  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    delete cac_pool;
  }
}
///------------------------------------------------------------------------------
}
