///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/mixin.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/slice.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
mixin::mixin(context& ctx, std::size_t id, attributes const& attrs)
  : basic_actor(attrs.max_cache_match_size_)
  , curr_cache_pool_(size_nil)
  , cache_pool_size_(attrs.per_mixin_cache_)
{
  basic_actor::update_aid();
  cache_pool_list_.reserve(attrs.per_mixin_cache_);

  try
  {
    std::size_t index = id;
    for (std::size_t i=0; i<attrs.per_mixin_cache_; ++i, ++index)
    {
      cache_pool_list_.push_back((detail::cache_pool*)0);
      detail::cache_pool*& cac_pool = cache_pool_list_.back();
      cac_pool = new detail::cache_pool(ctx, index, attrs, true);
    }

    owner_ = select_cache_pool();
    slice_pool_ =
      boost::in_place(
        owner_, this,
        attrs.slice_pool_free_size_,
        attrs.slice_pool_reserve_size_
        );
  }
  catch (...)
  {
    delete_cache();
    throw;
  }
}
///----------------------------------------------------------------------------
mixin::~mixin()
{
  delete_cache();
}
///----------------------------------------------------------------------------
aid_t mixin::recv(message& msg, match const& mach)
{
  detail::scope scp(boost::bind(&mixin::gc, this));
  aid_t sender;
  detail::recv_t rcv;

  basic_actor::move_pack(owner_);
  if (!mb_.pop(rcv, msg, mach.match_list_))
  {
    duration_t tmo = mach.timeout_;
    if (tmo > zero)
    {
      boost::unique_lock<boost::shared_mutex> lock(mtx_);
      basic_actor::move_pack(owner_);
      if (!mb_.pop(rcv, msg, mach.match_list_))
      {
        bool has_msg = false;
        duration_t curr_tmo = tmo;
        typedef boost::chrono::system_clock clock_t;
        clock_t::time_point begin_tp;
        do
        {
          boost::cv_status cv_stat = boost::cv_status::no_timeout;
          if (tmo == infin)
          {
            cv_.wait(lock);
          }
          else
          {
            begin_tp = clock_t::now();
            cv_stat = cv_.wait_for(lock, curr_tmo);
          }

          if (cv_stat == boost::cv_status::timeout)
          {
            return sender;
          }

          basic_actor::move_pack(owner_);
          has_msg = mb_.pop(rcv, msg, mach.match_list_);
          if (!has_msg && tmo != infin)
          {
            duration_t pass_time = clock_t::now() - begin_tp;
            curr_tmo -= pass_time;
          }
        }
        while (!has_msg);
      }
    }
    else
    {
      return sender;
    }
  }

  if (aid_t* aid = boost::get<aid_t>(&rcv))
  {
    sender = *aid;
  }
  else if (detail::request_t* req = boost::get<detail::request_t>(&rcv))
  {
    sender = req->get_aid();
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv))
  {
    sender = ex->get_aid();
  }

  return sender;
}
///----------------------------------------------------------------------------
void mixin::send(aid_t recver, message const& m)
{
  detail::pack* pk = basic_actor::alloc_pack(owner_);
  pk->tag_ = get_aid();
  pk->recver_ = recver;
  pk->msg_ = m;

  basic_actor* a = recver.get_actor_ptr();
  a->on_recv(pk);
}
///----------------------------------------------------------------------------
void mixin::reply(aid_t recver, message const& m)
{
  basic_actor* a = recver.get_actor_ptr();
  detail::request_t req;
  detail::pack* pk = basic_actor::alloc_pack(owner_);
  if (mb_.pop(recver, req))
  {
    response_t res(req.get_id(), get_aid());
    pk->tag_ = res;
    pk->recver_ = recver;
    pk->msg_ = m;
  }
  else
  {
    pk->tag_ = get_aid();
    pk->recver_ = recver;
    pk->msg_ = m;
  }
  a->on_recv(pk);
}
///----------------------------------------------------------------------------
void mixin::wait(duration_t dur)
{
  boost::this_thread::sleep_for(dur);
}
///------------------------------------------------------------------------------
detail::cache_pool* mixin::select_cache_pool()
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
///----------------------------------------------------------------------------
std::size_t mixin::get_cache_match_size() const
{
  return owner_->get_cache_match_size();
}
///----------------------------------------------------------------------------
void mixin::link(aid_t target)
{
  basic_actor::link(detail::link_t(linked, target), owner_);
}
///----------------------------------------------------------------------------
void mixin::monitor(aid_t target)
{
  basic_actor::link(detail::link_t(monitored, target), owner_);
}
///----------------------------------------------------------------------------
void mixin::on_recv(detail::pack* pk)
{
  boost::shared_lock<boost::shared_mutex> lock(mtx_);
  pack_que_.push(pk);
  cv_.notify_one();
}
///----------------------------------------------------------------------------
void mixin::gc()
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->free_cache();
      cac_pool->free_object();
    }
  }
}
///----------------------------------------------------------------------------
slice* mixin::get_slice()
{
  return slice_pool_->get();
}
///----------------------------------------------------------------------------
void mixin::free_slice(slice* t)
{
  slice_pool_->free(t);
}
///----------------------------------------------------------------------------
void mixin::free_cache()
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    if (cac_pool)
    {
      cac_pool->free_cache();
    }
  }
}
///----------------------------------------------------------------------------
void mixin::delete_cache()
{
  BOOST_FOREACH(detail::cache_pool* cac_pool, cache_pool_list_)
  {
    delete cac_pool;
  }
}
///----------------------------------------------------------------------------
}
