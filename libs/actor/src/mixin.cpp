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
#include <gce/actor/context.hpp>
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
  : base_type(attrs.max_cache_match_size_, ctx.get_timestamp())
  , ctx_(&ctx)
  , curr_cache_pool_(size_nil)
  , cache_pool_size_(attrs.per_mixin_cache_)
  , ctxid_(attrs.id_)
{
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
    user_ = select_cache_pool();
    base_type::update_aid(owner_->get_ctxid());
    slice_pool_ =
      boost::in_place(
        owner_, this,
        size_nil,
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

  move_pack(this, mb_, pack_que_, user_, this);
  if (!mb_.pop(rcv, msg, mach.match_list_))
  {
    duration_t tmo = mach.timeout_;
    if (tmo > zero)
    {
      boost::unique_lock<boost::shared_mutex> lock(mtx_);
      move_pack(this, mb_, pack_que_, user_, this);
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

          move_pack(this, mb_, pack_que_, user_, this);
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
    msg.req_ = *req;
  }
  else if (detail::exit_t* ex = boost::get<detail::exit_t>(&rcv))
  {
    sender = ex->get_aid();
  }

  return sender;
}
///----------------------------------------------------------------------------
aid_t mixin::recv(response_t res, message& msg, duration_t tmo)
{
  detail::scope scp(boost::bind(&mixin::gc, this));
  aid_t sender;

  move_pack(this, mb_, pack_que_, user_, this);
  if (!mb_.pop(res, msg))
  {
    if (tmo > zero)
    {
      boost::unique_lock<boost::shared_mutex> lock(mtx_);
      move_pack(this, mb_, pack_que_, user_, this);
      if (!mb_.pop(res, msg))
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

          move_pack(this, mb_, pack_que_, user_, this);
          has_msg = mb_.pop(res, msg);
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

  sender = res.get_aid();
  return sender;
}
///----------------------------------------------------------------------------
void mixin::wait(duration_t dur)
{
  boost::this_thread::sleep_for(dur);
}
///----------------------------------------------------------------------------
void mixin::link(aid_t target)
{
  base_type::link(detail::link_t(linked, target), user_);
}
///----------------------------------------------------------------------------
void mixin::monitor(aid_t target)
{
  base_type::link(detail::link_t(monitored, target), user_);
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
void mixin::move_pack(
  basic_actor* base,
  detail::mailbox& mb,
  detail::pack_queue_t& pack_que,
  detail::cache_pool* user,
  mixin* mix
  )
{
  detail::pack* pk = pack_que.pop_all();
  if (pk)
  {
    detail::scope scp(boost::bind(&basic_actor::dealloc_pack, user, pk));
    while (pk)
    {
      detail::pack* next = detail::node_access::get_next(pk);
      if (check(pk->recver_, base->get_aid().ctxid_, user->get_context().get_timestamp()))
      {
        if (aid_t* aid = boost::get<aid_t>(&pk->tag_))
        {
          match_t type = pk->msg_.get_type();
          if (type == detail::msg_reg_skt)
          {
            ctxid_t ctxid;
            aid_t skt;
            pk->msg_ >> ctxid >> skt;
            BOOST_FOREACH(detail::cache_pool* cac_pool, mix->cache_pool_list_)
            {
              if (cac_pool)
              {
                cac_pool->register_socket(ctxid, skt);
              }
            }
          }
          else if (type == detail::msg_dereg_skt)
          {
            ctxid_t ctxid;
            aid_t skt;
            pk->msg_ >> ctxid >> skt;
            BOOST_FOREACH(detail::cache_pool* cac_pool, mix->cache_pool_list_)
            {
              if (cac_pool)
              {
                cac_pool->deregister_socket(ctxid, skt);
              }
            }
          }
          else if (type == detail::msg_stop)
          {
            BOOST_FOREACH(detail::cache_pool* cac_pool, mix->cache_pool_list_)
            {
              if (cac_pool)
              {
                cac_pool->stop();
              }
            }
          }
          else
          {
            mb.push(*aid, pk->msg_);
          }
        }
        else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
        {
          mb.push(*req, pk->msg_);
        }
        else if (detail::exit_t* ex = boost::get<detail::exit_t>(&pk->tag_))
        {
          mb.push(*ex, pk->msg_);
          base->remove_link(ex->get_aid());
        }
        else if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
        {
          base->add_link(link->get_aid());
          return;
        }
        else if (response_t* res = boost::get<response_t>(&pk->tag_))
        {
          mb.push(*res, pk->msg_);
        }
      }
      else if (!pk->is_err_ret_)
      {
        if (detail::link_t* link = boost::get<detail::link_t>(&pk->tag_))
        {
          /// send actor exit msg
          base->send_already_exited(link->get_aid(), pk->recver_, user);
        }
        else if (detail::request_t* req = boost::get<detail::request_t>(&pk->tag_))
        {
          /// reply actor exit msg
          response_t res(req->get_id(), pk->recver_);
          base->send_already_exited(req->get_aid(), res, user);
        }
      }
      pk = next;
    }
  }
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
void mixin::register_socket(ctxid_t ctxid, aid_t skt, detail::cache_pool* user)
{
  detail::pack* pk = base_type::alloc_pack(user);
  aid_t self = get_aid();
  pk->tag_ = self;
  pk->recver_ = self;

  message m(detail::msg_reg_skt);
  m << ctxid << skt;
  pk->msg_ = m;
  on_recv(pk);
}
///----------------------------------------------------------------------------
void mixin::deregister_socket(ctxid_t ctxid, aid_t skt, detail::cache_pool* user)
{
  detail::pack* pk = base_type::alloc_pack(user);
  aid_t self = get_aid();
  pk->tag_ = self;
  pk->recver_ = self;

  message m(detail::msg_dereg_skt);
  m << ctxid << skt;
  pk->msg_ = m;
  on_recv(pk);
}
///----------------------------------------------------------------------------
void mixin::stop(detail::cache_pool* user)
{
  detail::pack* pk = base_type::alloc_pack(user);
  aid_t self = get_aid();
  pk->tag_ = self;
  pk->recver_ = self;

  pk->msg_ = message(detail::msg_stop);
  on_recv(pk);
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
