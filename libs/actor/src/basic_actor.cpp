///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/basic_actor.hpp>
#include <gce/actor/context.hpp>
#include <gce/actor/thread_mapped_actor.hpp>
#include <gce/actor/nonblocking_actor.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/detail/scope.hpp>
#include <boost/variant/get.hpp>
#include <boost/foreach.hpp>

namespace gce
{
///----------------------------------------------------------------------------
basic_actor::basic_actor(
  context* ctx, detail::cache_pool* user, 
  aid_t aid, std::size_t cache_queue_index
  )
  : ctx_(ctx)
  , user_(user)
  , snd_(user_->get_strand())
  , mb_(ctx_->get_attributes().max_cache_match_size_)
  , ctxid_(ctx_->get_attributes().id_)
  , timestamp_(ctx_->get_timestamp())
  , cache_queue_index_(cache_queue_index)
  , aid_(aid)
  , chain_(true)
  , req_id_(0)
{
}
///----------------------------------------------------------------------------
basic_actor::~basic_actor()
{
}
///----------------------------------------------------------------------------
void basic_actor::pri_send(aid_t recver, message const& m, detail::send_hint hint)
{
  aid_t target = user_->filter_aid(recver);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = get_aid();
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (!chain_)
    {
      hint = detail::async;
    }
    user_->send(target, pk, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_send_svc(svcid_t recver, message const& m, detail::send_hint hint)
{
  aid_t target = user_->filter_svcid(recver);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = get_aid();
    if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == ctxid_)
    {
      /// is local none socket actor
      pk.recver_ = target;
    }
    pk.svc_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (!chain_)
    {
      hint = detail::async;
    }
    user_->send(target, pk, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_relay(aid_t recver, message& m, detail::send_hint hint)
{
  aid_t target = user_->filter_aid(recver);
  if (target)
  {
    detail::pack pk;
    if (m.req_.valid())
    {
      pk.tag_ = m.req_;
      m.req_ = detail::request_t();
    }
    else
    {
      pk.tag_ = get_aid();
    }
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (!chain_)
    {
      hint = detail::async;
    }
    user_->send(target, pk, hint);
  }
  else if (m.req_.valid())
  {
    /// reply actor exit msg
    resp_t res(m.req_.get_id(), recver);
    user_->send_already_exited(m.req_.get_aid(), res);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_relay_svc(svcid_t recver, message& m, detail::send_hint hint)
{
  aid_t target = user_->filter_svcid(recver);
  if (target)
  {
    detail::pack pk;
    if (m.req_.valid())
    {
      pk.tag_ = m.req_;
      m.req_ = detail::request_t();
    }
    else
    {
      pk.tag_ = get_aid();
    }

    if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == ctxid_)
    {
      /// is local none socket actor
      pk.recver_ = target;
    }
    pk.svc_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (!chain_)
    {
      hint = detail::async;
    }
    user_->send(target, pk, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_request(resp_t res, aid_t recver, message const& m, detail::send_hint hint)
{
  aid_t target = user_->filter_aid(recver);
  aid_t sender = get_aid();
  detail::request_t req(res.get_id(), sender);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = req;
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (!chain_)
    {
      hint = detail::async;
    }
    user_->send(target, pk, hint);
  }
  else
  {
    /// reply actor exit msg
    resp_t res(req.get_id(), recver);
    user_->send_already_exited(req.get_aid(), res);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_request_svc(resp_t res, svcid_t recver, message const& m, detail::send_hint hint)
{
  aid_t target = user_->filter_svcid(recver);
  aid_t sender = get_aid();
  detail::request_t req(res.get_id(), sender);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = req;
    if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == ctxid_)
    {
      /// is local none socket actor
      pk.recver_ = target;
    }
    pk.svc_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (!chain_)
    {
      hint = detail::async;
    }
    user_->send(target, pk, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_reply(aid_t recver, message const& m, detail::send_hint hint)
{
  aid_t target = user_->filter_aid(recver);
  if (target)
  {
    detail::request_t req;
    detail::pack pk;
    if (mb_.pop(recver, req))
    {
      resp_t res(req.get_id(), get_aid());
      pk.tag_ = res;
    }
    else
    {
      pk.tag_ = get_aid();
    }
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (!chain_)
    {
      hint = detail::async;
    }
    user_->send(target, pk, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_link(aid_t target, detail::send_hint hint)
{
  link(detail::link_t(linked, target), hint, user_);
}
///----------------------------------------------------------------------------
void basic_actor::pri_monitor(aid_t target, detail::send_hint hint)
{
  link(detail::link_t(monitored, target), hint, user_);
}
///----------------------------------------------------------------------------
void basic_actor::pri_spawn(
  sid_t sid, detail::spawn_type type, std::string const& func, match_t ctxid,
  std::size_t stack_size, detail::send_hint hint
  )
{
  ctxid_t target;
  aid_t skt = user_->select_socket(ctxid, &target);
  if (!skt)
  {
    throw std::runtime_error("no socket available");
  }

  if (ctxid == ctxid_nil)
  {
    ctxid = target;
  }

  detail::pack pk;
  pk.tag_ = detail::spawn_t(type, func, ctxid, stack_size, sid, get_aid());
  pk.skt_ = skt;
  pk.msg_ = message(detail::msg_spawn);
  pk.recver_.ctxid_ = ctxid_nil;

  if (!chain_)
  {
    hint = detail::async;
  }
  user_->send(skt, pk, hint);
}
///----------------------------------------------------------------------------
void basic_actor::on_free()
{
  mb_.clear();
  link_list_.clear();
  monitor_list_.clear();
}
///----------------------------------------------------------------------------
sid_t basic_actor::new_request()
{
  return ++req_id_;
}
///----------------------------------------------------------------------------
void basic_actor::add_link(aid_t target, sktaid_t skt)
{
  link_list_.insert(std::make_pair(target, skt));
}
///----------------------------------------------------------------------------
void basic_actor::link(detail::link_t l, detail::send_hint hint, detail::cache_pool* user)
{
  aid_t recver = l.get_aid();
  aid_t skt;
  bool is_local = check_local(recver, ctxid_);
  if (!is_local)
  {
    BOOST_ASSERT(user);
    skt = user->select_socket(recver.ctxid_);
    if (!skt)
    {
      user->send_already_exited(get_aid(), recver);
      return;
    }
  }

  if (l.get_type() == linked)
  {
    add_link(recver, skt);
  }
  else
  {
    monitor_list_.insert(recver);
  }

  if (user)
  {
    aid_t target = is_local ? recver : skt;
    BOOST_ASSERT(target);

    detail::pack pk;
    pk.tag_ = detail::link_t(l.get_type(), get_aid());
    pk.recver_ = recver;
    pk.skt_ = skt;
    pk.msg_ = message(detail::msg_link);

    if (!chain_)
    {
      hint = detail::async;
    }
    user->send(target, pk, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::send_exit(
  aid_t self_aid,
  exit_code_t ec,
  std::string const& exit_msg
  )
{
  message m(exit);
  m << ec << exit_msg;

  BOOST_FOREACH(link_list_t::value_type& pr, link_list_)
  {
    aid_t target = pr.second ? pr.second : pr.first;
    BOOST_ASSERT(target);

    detail::pack pk;
    pk.tag_ = detail::exit_t(ec, self_aid);
    pk.recver_ = pr.first;
    pk.skt_ = target;
    pk.msg_ = m;

    user_->send(target, pk, detail::async);
  }
}
///----------------------------------------------------------------------------
void basic_actor::remove_link(aid_t aid)
{
  link_list_.erase(aid);
  monitor_list_.erase(aid);
}
///----------------------------------------------------------------------------
}
