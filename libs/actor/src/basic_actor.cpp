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
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/detail/scope.hpp>
#include <boost/variant/get.hpp>
#include <boost/foreach.hpp>

namespace gce
{
///----------------------------------------------------------------------------
basic_actor::basic_actor(std::size_t cache_match_size, timestamp_t const timestamp)
  : owner_(0)
  , user_(0)
  , mb_(cache_match_size)
  , chain_(false)
  , req_id_(0)
  , timestamp_(timestamp)
{
  aid_ = aid_t(ctxid_nil, timestamp, this, 0);
}
///----------------------------------------------------------------------------
basic_actor::~basic_actor()
{
}
///----------------------------------------------------------------------------
void basic_actor::pri_send(aid_t recver, message const& m, send_hint hint)
{
  aid_t target = filter_aid(recver, user_);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = get_aid();
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user_, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_send_svc(svcid_t recver, message const& m, send_hint hint)
{
  aid_t target = filter_svcid(recver, user_);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = get_aid();
    if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == user_->get_ctxid())
    {
      /// is local none socket actor
      pk.recver_ = target;
    }
    pk.svc_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user_, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_relay(aid_t recver, message& m, send_hint hint)
{
  aid_t target = filter_aid(recver, user_);
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

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user_, hint);
  }
  else if (m.req_.valid())
  {
    /// reply actor exit msg
    response_t res(m.req_.get_id(), recver);
    send_already_exited(m.req_.get_aid(), res);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_relay_svc(svcid_t recver, message& m, send_hint hint)
{
  aid_t target = filter_svcid(recver, user_);
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

    if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == user_->get_ctxid())
    {
      /// is local none socket actor
      pk.recver_ = target;
    }
    pk.svc_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user_, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_request(response_t res, aid_t recver, message const& m, send_hint hint)
{
  aid_t target = filter_aid(recver, user_);
  aid_t sender = get_aid();
  detail::request_t req(res.get_id(), sender);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = req;
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user_, hint);
  }
  else
  {
    /// reply actor exit msg
    response_t res(req.get_id(), recver);
    send_already_exited(req.get_aid(), res);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_request_svc(response_t res, svcid_t recver, message const& m, send_hint hint)
{
  aid_t target = filter_svcid(recver, user_);
  aid_t sender = get_aid();
  detail::request_t req(res.get_id(), sender);
  if (target)
  {
    detail::pack pk;
    pk.tag_ = req;
    if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == user_->get_ctxid())
    {
      /// is local none socket actor
      pk.recver_ = target;
    }
    pk.svc_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user_, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_reply(aid_t recver, message const& m, send_hint hint)
{
  aid_t target = filter_aid(recver, user_);
  if (target)
  {
    detail::request_t req;
    detail::pack pk;
    if (mb_.pop(recver, req))
    {
      response_t res(req.get_id(), get_aid());
      pk.tag_ = res;
    }
    else
    {
      pk.tag_ = get_aid();
    }
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user_, hint);
  }
}
///----------------------------------------------------------------------------
void basic_actor::pri_link(aid_t target, send_hint hint)
{
  link(detail::link_t(linked, target), hint, user_);
}
///----------------------------------------------------------------------------
void basic_actor::pri_monitor(aid_t target, send_hint hint)
{
  link(detail::link_t(monitored, target), hint, user_);
}
///----------------------------------------------------------------------------
void basic_actor::pri_spawn(
  sid_t sid, match_t func, match_t ctxid,
  std::size_t stack_size, send_hint hint
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
  pk.tag_ = detail::spawn_t(func, ctxid, stack_size, sid, get_aid());
  pk.skt_ = skt;
  pk.msg_ = message(detail::msg_spawn);

  if (chain_)
  {
    hint = sync;
  }
  send(skt, pk, user_, hint);
}
///----------------------------------------------------------------------------
void basic_actor::on_free()
{
  mb_.clear();
  link_list_.clear();
  monitor_list_.clear();
}
///----------------------------------------------------------------------------
void basic_actor::update_aid()
{
  BOOST_ASSERT(user_);
  aid_ = aid_t(user_->get_ctxid(), aid_.timestamp_, this, aid_.sid_ + 1);
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
void basic_actor::link(detail::link_t l, send_hint hint, detail::cache_pool* user)
{
  aid_t recver = l.get_aid();
  aid_t skt;
  bool is_local = check_local(recver, user_->get_ctxid());
  if (!is_local)
  {
    BOOST_ASSERT(user);
    skt = user->select_socket(recver.ctxid_);
    if (!skt)
    {
      send_already_exited(get_aid(), recver);
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

    if (chain_)
    {
      hint = sync;
    }
    send(target, pk, user, hint);
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

    send(target, pk, user_, async);
  }
}
///----------------------------------------------------------------------------
void basic_actor::remove_link(aid_t aid)
{
  link_list_.erase(aid);
  monitor_list_.erase(aid);
}
///----------------------------------------------------------------------------
void basic_actor::send_already_exited(aid_t recver, aid_t sender)
{
  aid_t target = filter_aid(recver, user_);
  if (target)
  {
    message m(exit);
    std::string exit_msg("already exited");
    m << exit_already << exit_msg;

    detail::pack pk;
    pk.tag_ = sender;
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;
    pk.is_err_ret_ = true;

    send(target, pk, user_, async);
  }
}
///----------------------------------------------------------------------------
void basic_actor::send_already_exited(aid_t recver, response_t res)
{
  aid_t target = filter_aid(recver, user_);
  if (target)
  {
    message m(exit);
    std::string exit_msg("already exited");
    m << exit_already << exit_msg;

    detail::pack pk;
    pk.tag_ = res;
    pk.recver_ = recver;
    pk.skt_ = target;
    pk.msg_ = m;
    pk.is_err_ret_ = true;

    send(target, pk, user_, async);
  }
}
///----------------------------------------------------------------------------
void basic_actor::send(
  aid_t const& recver, detail::pack& pk,
  detail::cache_pool* user, send_hint hint
  )
{
  recver.get_actor_ptr(
    user->get_ctxid(),
    user->get_context().get_timestamp()
    )->on_recv(pk, hint);
}
///----------------------------------------------------------------------------
aid_t basic_actor::filter_aid(aid_t const& src, detail::cache_pool* user)
{
  aid_t target;
  ctxid_t ctxid = user->get_ctxid();
  timestamp_t timestamp = user->get_context().get_timestamp();

  bool is_local = check_local(src, ctxid);
  if (is_local && check_local_valid(src, ctxid, timestamp))
  {
    target = src;
  }
  else
  {
    if (!is_local)
    {
      target = user->select_socket(src.ctxid_);
    }
  }
  return target;
}
///----------------------------------------------------------------------------
aid_t basic_actor::filter_svcid(svcid_t const& src, detail::cache_pool* user)
{
  aid_t target;
  ctxid_t ctxid = user->get_ctxid();

  if (src.ctxid_ == ctxid_nil || src.ctxid_ == ctxid)
  {
    target = user->find_service(src.name_);
  }
  else
  {
    target = user->select_socket(src.ctxid_);
  }
  return target;
}
///----------------------------------------------------------------------------
}
