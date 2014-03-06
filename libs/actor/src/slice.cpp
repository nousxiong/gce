///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/slice.hpp>
#include <gce/actor/mixin.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/message.hpp>
#include <gce/detail/scope.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/variant/get.hpp>

namespace gce
{
///----------------------------------------------------------------------------
slice::slice(mixin* sire)
  : basic_actor(
      sire->get_context().get_attributes().max_cache_match_size_,
      sire->get_context().get_timestamp()
      )
  , detail::ref_count_st(boost::bind(&slice::free, this))
  , sire_(sire)
{
  owner_ = sire_->get_cache_pool();
  user_ = sire_->get_cache_pool();
}
///----------------------------------------------------------------------------
slice::~slice()
{
}
///----------------------------------------------------------------------------
aid_t slice::recv(message& msg, match_list_t const& match_list)
{
  aid_t sender;
  detail::recv_t rcv;

  mixin::move_pack(this, mb_, pack_que_, user_, sire_);
  if (!mb_.pop(rcv, msg, match_list))
  {
    return sender;
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
aid_t slice::recv(response_t res, message& msg)
{
  aid_t sender;

  mixin::move_pack(this, mb_, pack_que_, user_, sire_);
  if (!mb_.pop(res, msg))
  {
    return sender;
  }

  sender = res.get_aid();
  return sender;
}
///----------------------------------------------------------------------------
void slice::link(aid_t target)
{
  basic_actor::link(detail::link_t(linked, target), user_);
}
///----------------------------------------------------------------------------
void slice::monitor(aid_t target)
{
  basic_actor::link(detail::link_t(monitored, target), user_);
}
///----------------------------------------------------------------------------
void slice::init(aid_t link_tgt)
{
  base_type::update_aid();
  if (link_tgt)
  {
    base_type::add_link(link_tgt);
  }
}
///----------------------------------------------------------------------------
void slice::on_free()
{
  base_type::on_free();
}
///----------------------------------------------------------------------------
void slice::on_recv(detail::pack* pk)
{
  pack_que_.push(pk);
}
///----------------------------------------------------------------------------
void slice::free()
{
  base_type::send_exit(exit_normal, "exit normal");
  base_type::update_aid();
  sire_->free_slice(this);
}
///----------------------------------------------------------------------------
}

