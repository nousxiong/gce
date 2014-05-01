///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/context.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/spawn.hpp>
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
  , curr_thread_(size_nil)
  , thread_size_(attrs_.thread_num_ == 0 ? 1 : attrs_.thread_num_)
{
  if (attrs_.ios_)
  {
    ios_.reset(attrs_.ios_, detail::empty_deleter<io_service_t>());
  }
  else
  {
    ios_.reset(new io_service_t);
  }

  try
  {
    for (std::size_t i=0; i<thread_size_; ++i)
    {
      thread_list_.emplace_back(*this, i);
    }

    for (std::size_t i=0; i<thread_size_; ++i)
    {
      thread& thr = thread_list_[i];
      thread_group_.create_thread(
        boost::bind(
          &thread::run, &thr,
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
  mixin* mix = new mixin(&select_thread(), attrs_);
  mixin_list_.push(mix);
  return *mix;
}
///------------------------------------------------------------------------------
thread& context::select_thread()
{
  std::size_t curr_thread = curr_thread_;
  ++curr_thread;
  if (curr_thread >= thread_size_)
  {
    curr_thread = 0;
  }
  curr_thread_ = curr_thread;
  return thread_list_[curr_thread];
}
///------------------------------------------------------------------------------
void context::register_service(match_t name, aid_t svc)
{
  BOOST_FOREACH(thread& thr, thread_list_)
  {
    thr.get_io_service().dispatch(
      boost::bind(
        &detail::cache_pool::register_service,
        &thr.get_cache_pool(), name, svc
        )
      );
  }
}
///------------------------------------------------------------------------------
void context::deregister_service(match_t name, aid_t svc)
{
  BOOST_FOREACH(thread& thr, thread_list_)
  {
    thr.get_io_service().dispatch(
      boost::bind(
        &detail::cache_pool::deregister_service,
        &thr.get_cache_pool(), name, svc
        )
      );
  }
}
///------------------------------------------------------------------------------
void context::register_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  BOOST_FOREACH(thread& thr, thread_list_)
  {
    thr.get_io_service().dispatch(
      boost::bind(
        &detail::cache_pool::register_socket,
        &thr.get_cache_pool(), ctxid_pr, skt
        )
      );
  }
}
///------------------------------------------------------------------------------
void context::deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt)
{
  BOOST_FOREACH(thread& thr, thread_list_)
  {
    thr.get_io_service().dispatch(
      boost::bind(
        &detail::cache_pool::deregister_socket,
        &thr.get_cache_pool(), ctxid_pr, skt
        )
      );
  }
}
///------------------------------------------------------------------------------
void context::stop()
{
  BOOST_FOREACH(thread& thr, thread_list_)
  {
    thr.get_io_service().dispatch(
      boost::bind(
        &thread::stop, &thr
        )
      );
  }

  thread_group_.join_all();

  mixin* mix = mixin_list_.pop_all_reverse();
  while (mix)
  {
    mixin* next = detail::node_access::get_next(mix);
    detail::node_access::set_next(mix, (mixin*)0);
    delete mix;
    mix = next;
  }

  thread_list_.clear();
}
///------------------------------------------------------------------------------
}
