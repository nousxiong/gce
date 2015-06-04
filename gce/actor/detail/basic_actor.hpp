///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_BASIC_ACTOR_HPP
#define GCE_ACTOR_DETAIL_BASIC_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/service_id.hpp>
#include <gce/actor/response.hpp>
#include <gce/actor/detail/basic_service.hpp>
#include <gce/actor/detail/mailbox.hpp>
#include <gce/actor/detail/request.hpp>
#include <gce/actor/detail/link.hpp>
#include <gce/actor/detail/listener.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/actor_fwd.hpp>
#include <gce/actor/actor_id.hpp>
#include <boost/function.hpp>
#include <map>
#include <set>

namespace gce
{
namespace detail
{
template <typename Context>
class basic_actor
  : public listener
{
  typedef Context context_t;
  typedef basic_service<context_t> service_t;

public:
  basic_actor(context_t& ctx, service_t& svc, actor_type type, aid_t aid)
    : ctx_(ctx)
    , basic_svc_(svc)
    , snd_(basic_svc_.get_strand())
    , mb_(ctx_.get_attributes().max_cache_match_size_)
    , ctxid_(ctx_.get_ctxid())
    , timestamp_(ctx_.get_timestamp())
    , type_(type)
    , aid_(aid)
    , req_id_(0)
    , lg_(ctx_.get_logger())
  {
  }
  
  virtual ~basic_actor()
  {
  }

public:
  context_t& get_context()
  {
    return ctx_; 
  }

  service_t& get_basic_service()
  {
    return basic_svc_;
  }

  strand_t& get_strand() 
  {
    return snd_; 
  }

  aid_t const& get_aid() const 
  { 
    return aid_; 
  }

  actor_type get_type() const
  {
    return type_;
  }

  log::logger_t& get_logger()
  {
    return lg_;
  }

public:
  void pri_send(aid_t const& recver, message const& m)
  {
    aid_t target = basic_svc_.filter_aid(recver);
    if (target != aid_nil)
    {
      pack& pk = basic_svc_.alloc_pack(target);
      pk.tag_ = get_aid();
      pk.recver_ = recver;
      pk.skt_ = target;
      pk.msg_ = m;

      basic_svc_.send(target, pk);
    }
  }

  void pri_send_svc(svcid_t const& recver, message const& m)
  {
    aid_t target = basic_svc_.filter_svcid(recver);
    if (target != aid_nil)
    {
      pack& pk = basic_svc_.alloc_pack(target);
      pk.tag_ = get_aid();
      if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == ctxid_)
      {
        /// is local none socket actor
        pk.recver_ = target;
      }
      pk.svc_ = recver;
      pk.skt_ = target;
      pk.msg_ = m;

      basic_svc_.send(target, pk);
    }
  }

  void pri_send_svcs(match_t recver, message const& m)
  {
    svcid_t svcid = basic_svc_.filter_svcid(recver);
    if (svcid != svcid_nil)
    {
      pri_send_svc(svcid, m);
    }
  }

  void pri_relay(aid_t const& recver, message& m)
  {
    aid_t target = basic_svc_.filter_aid(recver);
    request_t const* req = m.get_relay<request_t>();
    if (target != aid_nil)
    {
      pack& pk = basic_svc_.alloc_pack(target);
      if (req)
      {
        GCE_ASSERT(req->valid())(recver);
        pk.tag_ = *req;
        m.clear_relay();
      }
      else if (aid_t const* aid = m.get_relay<aid_t>())
      {
        GCE_ASSERT(*aid != aid_nil)(recver);
        GCE_ASSERT(aid->type_ != (byte_t)actor_addon)(recver);
        pk.tag_ = *aid;
        m.clear_relay();
      }
      else
      {
        pk.tag_ = get_aid();
      }
      pk.recver_ = recver;
      pk.skt_ = target;
      pk.msg_ = m;

      basic_svc_.send(target, pk);
    }
    else if (req && req->valid())
    {
      /// reply actor exit msg
      resp_t res(req->get_id(), recver);
      basic_svc_.send_already_exited(req->get_aid(), res);
    }
  }

  void pri_relay_svc(svcid_t const& recver, message& m)
  {
    aid_t target = basic_svc_.filter_svcid(recver);
    if (target != aid_nil)
    {
      pack& pk = basic_svc_.alloc_pack(target);
      if (request_t const* req = m.get_relay<request_t>())
      {
        GCE_ASSERT(req->valid())(recver);
        pk.tag_ = *req;
        m.clear_relay();
      }
      else if (aid_t const* aid = m.get_relay<aid_t>())
      {
        GCE_ASSERT(*aid != aid_nil)(recver);
        GCE_ASSERT(aid->type_ != (byte_t)actor_addon)(recver);
        pk.tag_ = *aid;
        m.clear_relay();
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

      basic_svc_.send(target, pk);
    }
  }

  void pri_relay_svcs(match_t recver, message& m)
  {
    svcid_t svcid = basic_svc_.filter_svcid(recver);
    if (svcid != svcid_nil)
    {
      pri_relay_svc(svcid, m);
    }
  }

  void pri_request(resp_t const& res, aid_t recver, message const& m)
  {
    aid_t target = basic_svc_.filter_aid(recver);
    aid_t sender = get_aid();
    request_t req(res.get_id(), sender);
    if (target != aid_nil)
    {
      pack& pk = basic_svc_.alloc_pack(target);
      pk.tag_ = req;
      pk.recver_ = recver;
      pk.skt_ = target;
      pk.msg_ = m;

      basic_svc_.send(target, pk);
    }
    else
    {
      /// reply actor exit msg
      resp_t res(req.get_id(), recver);
      basic_svc_.send_already_exited(req.get_aid(), res);
    }
  }

  void pri_request_svc(resp_t const& res, svcid_t recver, message const& m)
  {
    aid_t target = basic_svc_.filter_svcid(recver);
    aid_t sender = get_aid();
    request_t req(res.get_id(), sender);
    if (target != aid_nil)
    {
      pack& pk = basic_svc_.alloc_pack(target);
      pk.tag_ = req;
      if (recver.ctxid_ == ctxid_nil || recver.ctxid_ == ctxid_)
      {
        /// is local none socket actor
        pk.recver_ = target;
      }
      pk.svc_ = recver;
      pk.skt_ = target;
      pk.msg_ = m;

      basic_svc_.send(target, pk);
    }
  }

  void pri_request_svcs(resp_t& res, match_t recver, message const& m)
  {
    svcid_t svcid = basic_svc_.filter_svcid(recver);
    if (svcid != svcid_nil)
    {
      res.set_recver(svcid);
      pri_request_svc(res, svcid, m);
    }
  }

  void pri_reply(aid_t const& recver, message const& m)
  {
    aid_t target = basic_svc_.filter_aid(recver);
    if (target != aid_nil)
    {
      request_t req;
      pack& pk = basic_svc_.alloc_pack(target);
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

      basic_svc_.send(target, pk);
    }
  }

  void pri_link(aid_t const& target)
  {
    link(link_t(linked, target), &basic_svc_);
  }

  void pri_monitor(aid_t const& target)
  {
    link(link_t(monitored, target), &basic_svc_);
  }

  void pri_spawn(
    sid_t sid, spawn_type type, std::string const& func, 
    match_t ctxid, size_t stack_size
    )
  {
    ctxid_t target;
    aid_t skt = basic_svc_.select_socket(ctxid, &target);
    GCE_VERIFY(skt != aid_nil)(ctxid)(target).msg("no socket available");

    if (ctxid == ctxid_nil)
    {
      ctxid = target;
    }

    pack& pk = basic_svc_.alloc_pack(skt);
    pk.tag_ = spawn_t(type, func, ctxid, stack_size, sid, get_aid());
    pk.skt_ = skt;
    pk.msg_ = message(msg_spawn);
    pk.recver_.ctxid_ = ctxid_nil;

    basic_svc_.send(skt, pk);
  }

protected:
  sid_t new_request()
  {
    return ++req_id_;
  }

  void add_link(aid_t const& target, sktaid_t skt = aid_t())
  {
    link_list_.insert(std::make_pair(target, skt));
  }

  void link(link_t l, service_t* svc = 0)
  {
    aid_t recver = l.get_aid();
    aid_t skt;
    bool is_local = check_local(recver, ctxid_);
    if (!is_local)
    {
      GCE_ASSERT(svc)(recver);
      skt = svc->select_socket(recver.ctxid_);
      if (skt == aid_nil)
      {
        svc->send_already_exited(get_aid(), recver);
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

    if (svc)
    {
      aid_t target = is_local ? recver : skt;
      GCE_ASSERT(target != aid_nil)(recver);

      pack& pk = svc->alloc_pack(target);
      pk.tag_ = link_t(l.get_type(), get_aid());
      pk.recver_ = recver;
      pk.skt_ = skt;
      pk.msg_ = message(msg_link);

      svc->send(target, pk);
    }
  }

  void send_exit(aid_t const& self_aid, exit_code_t ec, std::string const& exit_msg)
  {
    message m(exit);
    m << ec << exit_msg;

    BOOST_FOREACH(link_list_t::value_type& pr, link_list_)
    {
      aid_t const& target = pr.second != aid_nil ? pr.second : pr.first;
      GCE_ASSERT(target != aid_nil)(pr.first);

      pack& pk = basic_svc_.alloc_pack(target);
      pk.tag_ = exit_t(ec, self_aid);
      pk.recver_ = pr.first;
      pk.skt_ = target;
      pk.msg_ = m;

      basic_svc_.send(target, pk);
    }
  }

  void remove_link(aid_t const& aid)
  {
    link_list_.erase(aid);
    monitor_list_.erase(aid);
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

protected:
  GCE_CACHE_ALIGNED_VAR(context_t&, ctx_)
  GCE_CACHE_ALIGNED_VAR(service_t&, basic_svc_)
  GCE_CACHE_ALIGNED_VAR(strand_t&, snd_)
  GCE_CACHE_ALIGNED_VAR(mailbox, mb_)
  GCE_CACHE_ALIGNED_VAR(ctxid_t const, ctxid_)
  GCE_CACHE_ALIGNED_VAR(timestamp_t const, timestamp_)

private:
  GCE_CACHE_ALIGNED_VAR(actor_type, type_)
  GCE_CACHE_ALIGNED_VAR(aid_t, aid_)

  /// local vals
  sid_t req_id_;
  typedef std::map<aid_t, sktaid_t> link_list_t;
  typedef std::set<aid_t> monitor_list_t;
  link_list_t link_list_;
  monitor_list_t monitor_list_;
  log::logger_t& lg_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_BASIC_ACTOR_HPP
