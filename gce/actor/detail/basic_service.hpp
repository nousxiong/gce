///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_BASIC_SERVICE_HPP
#define GCE_ACTOR_DETAIL_BASIC_SERVICE_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/detail/listener.hpp>
#include <set>
#include <map>

namespace gce
{
namespace detail
{
struct pack;
class send_pair;
template <typename Context>
class basic_service
{
  typedef Context context_t;
  typedef basic_service<context_t> self_t;

public:
  basic_service(context_t& ctx, strand_t& snd, size_t index, actor_type type)
    : ctx_(ctx)
    , index_(index)
    , type_(type)
    , snd_(snd)
    , ctxid_(ctx_.get_ctxid())
    , timestamp_(ctx_.get_timestamp())
    , stopped_(false)
  {
  }

public:
  virtual void on_recv_forth(self_t& sender_svc, send_pair& sp) {};
  virtual void on_recv_back(self_t& sender_svc, send_pair& sp) {};
  virtual void on_recv(pack&) {};

  virtual pack& alloc_pack(aid_t const& target) = 0;

  void send(aid_t const& recver, pack& pk)
  {
    GCE_ASSERT(recver.type_ != (byte_t)actor_addon);
    pk.concurrency_index_ = index_;
    pk.type_ = type_;
    bool already_exit = true;
    if (in_pool(recver))
    {
      actor_index ai = get_actor_index(recver, ctxid_, timestamp_);
      if (ai)
      {
        already_exit = false;
        pk.ai_ = ai;
        pk.sid_ = recver.sid_;

        pri_send(ai, pk);
      }
    }
    else
    {
      listener* a = get_actor_ptr(recver, ctxid_, timestamp_);
      if (a)
      {
        already_exit = false;
        a->on_recv(pk);
      }
    }

    if (already_exit)
    {
      pk.expiry_ = true;
      send_already_exit(pk);
    }
  }

  void send(listener* a, pack& pk)
  {
    pk.concurrency_index_ = index_;
    pk.type_ = type_;
    if (a)
    {
      a->on_addon_recv(pk);
    }
  }

protected:
  virtual void pri_send(actor_index, pack&) = 0;

  void send2skt(aid_t const& from, aid_t const& to, message const& m)
  {
    GCE_ASSERT(to.type_ == (byte_t)actor_socket);
    pack& pk = alloc_pack(to);
    pk.tag_ = from;
    pk.recver_ = to;
    pk.skt_ = to;
    pk.msg_ = m;

    send(to, pk);
  }

public:
  context_t& get_context()
  {
    return ctx_;
  }

  size_t get_index() const
  {
    return index_;
  }

  actor_type get_type() const
  {
    return type_;
  }

  strand_t& get_strand()
  {
    return snd_;
  }

public:
  void register_service(match_t name, aid_t const& svc)
  {
    std::pair<service_list_t::iterator, bool> pr = 
      service_list_.insert(std::make_pair(name, svc));
    if (!pr.second && pr.first->second != svc)
    {
      pr.first->second = svc;
    }
  }

  aid_t find_service(match_t name)
  {
    aid_t svc;
    service_list_t::iterator itr(service_list_.find(name));
    if (itr != service_list_.end())
    {
      svc = itr->second;
    }
    return svc;
  }

  void deregister_service(match_t name, aid_t const& svc)
  {
    service_list_t::iterator itr(service_list_.find(name));
    if (itr != service_list_.end() && itr->second == svc)
    {
      service_list_.erase(itr);
    }
  }

  void add_service(match_t name, ctxid_t ctxid)
  {
    std::pair<typename global_service_list_t::iterator, bool> pr =
      global_service_list_.insert(std::make_pair(name, ctx_dummy_));
    std::pair<global_ctxid_list_t::iterator, bool> svcitr_pr =
      global_ctxid_list_.insert(std::make_pair(ctxid, svcitr_dummy_));
    svcitr_pr.first->second.insert(std::make_pair(name, &pr.first->second.ctx_list_));

    ctx_list_t& ctx_list = pr.first->second.ctx_list_;
    std::pair<ctx_list_t::iterator, bool> ctx_pr = ctx_list.insert(ctxid);

    if (ctx_list.size() == 1)
    {
      pr.first->second.curr_ctx_ = ctx_list.begin();
    }
  }

  bool has_service(match_t name, ctxid_t ctxid) const
  {
    typename global_service_list_t::const_iterator itr(global_service_list_.find(name));
    if (itr != global_service_list_.end())
    {
      ctx_list_t const& ctx_list = itr->second.ctx_list_;
      if (ctx_list.find(ctxid) != ctx_list.end())
      {
        return true;
      }
    }
    return false;
  }

  void rmv_service(match_t name, ctxid_t ctxid)
  {
    typename global_service_list_t::iterator itr(global_service_list_.find(name));
    if (itr != global_service_list_.end())
    {
      global_ctxid_list_t::iterator ctxid_itr(global_ctxid_list_.find(ctxid));
      GCE_ASSERT(ctxid_itr != global_ctxid_list_.end());
      ctxid_itr->second.erase(name);
      if (ctxid_itr->second.empty())
      {
        global_ctxid_list_.erase(ctxid_itr);
      }

      ctxid_list& ctx_list = itr->second;
      ctx_list_t::iterator ctx_itr = ctx_list.ctx_list_.find(ctxid);
      ctx_list_t::iterator next_itr(ctx_itr);
      bool is_curr = ctx_itr == ctx_list.curr_ctx_;
      ++next_itr;
      ctx_list.ctx_list_.erase(ctx_itr);
      if (is_curr)
      {
        ctx_list.curr_ctx_ = next_itr;
      }

      if (ctx_list.ctx_list_.empty())
      {
        global_service_list_.erase(itr);
      }
    }
  }

  void broadcast_add_service(aid_t const& from, adl::detail::add_svc const& svcs)
  {
    message m(msg_add_svc);
    m << svcs;

    /// send to all socket
    broadcast2socket(conn_list_, from, m);
    broadcast2socket(joint_list_, from, m);
    broadcast2socket(router_list_, from, m);
  }

  void broadcast_add_service(aid_t const& from, match_t name, ctxid_t ctxid)
  {
    tmp_add_svc_.svcs_.clear();
    adl::detail::svc_pair sp;
    sp.name_ = name;
    sp.ctxid_ = ctxid;
    tmp_add_svc_.svcs_.push_back(sp);
    broadcast_add_service(from, tmp_add_svc_);
  }

  void broadcast_rmv_service(aid_t const& from, adl::detail::rmv_svc const& svcs)
  {
    message m(msg_rmv_svc);
    m << svcs;

    /// send to all socket
    broadcast2socket(conn_list_, from, m);
    broadcast2socket(joint_list_, from, m);
    broadcast2socket(router_list_, from, m);
  }

  void broadcast_rmv_service(aid_t const& from, match_t name, ctxid_t ctxid)
  {
    tmp_rmv_svc_.names_.clear();
    tmp_rmv_svc_.ctxid_ = ctxid;
    tmp_rmv_svc_.names_.push_back(name);
    broadcast_rmv_service(from, tmp_rmv_svc_);
  }

  void get_global_service_list(adl::detail::global_service_list& glb_svc_list) const
  {
    typedef std::map<match_t, adl::detail::ctxid_list> adl_global_service_list_t;
    BOOST_FOREACH(typename global_service_list_t::value_type const& pr, global_service_list_)
    {
      std::pair<adl_global_service_list_t::iterator, bool> rt = 
        glb_svc_list.list_.insert(std::make_pair(pr.first, adl_ctx_dummy_));
      GCE_ASSERT(rt.second);
      ctx_list_t const& ctx_list = pr.second.ctx_list_;
      std::vector<match_t>& vec = rt.first->second.list_;
      vec.resize(ctx_list.size());
      std::copy(ctx_list.begin(), ctx_list.end(), vec.begin());
    }
  }

  void merge_global_service_list(aid_t const& from, adl::detail::global_service_list const& glb_svc_list)
  {
    typedef std::pair<match_t, adl::detail::ctxid_list> pair_t;
    tmp_add_svc_.svcs_.clear();
    adl::detail::svc_pair sp;
    BOOST_FOREACH(pair_t const& pr, glb_svc_list.list_)
    {
      BOOST_FOREACH(ctxid_t const& ctxid, pr.second.list_)
      {
        if (ctxid != ctxid_)
        {
          match_t const& name = pr.first;
          if (!has_service(name, ctxid))
          {
            ctx_.add_service(name, ctxid, get_type(), get_index());

            sp.name_ = name;
            sp.ctxid_ = ctxid;
            tmp_add_svc_.svcs_.push_back(sp);
          }
        }
      }
    }

    if (!tmp_add_svc_.svcs_.empty())
    {
      broadcast_add_service(from, tmp_add_svc_);
    }
  }

  void rmv_peer_services(aid_t const& from, ctxid_t peer)
  {
    if (peer == ctxid_nil || peer == ctxid_)
    {
      return;
    }

    tmp_rmv_svc_.ctxid_ = peer;
    tmp_rmv_svc_.names_.clear();
    global_ctxid_list_t::iterator itr(global_ctxid_list_.find(peer));
    if (itr != global_ctxid_list_.end())
    {
      BOOST_FOREACH(global_svcitr_list_t::value_type& pr, itr->second)
      {
        pr.second->erase(peer);
        tmp_rmv_svc_.names_.push_back(pr.first);
      }
      global_ctxid_list_.erase(itr);
    }

    if (!tmp_rmv_svc_.names_.empty())
    {
      broadcast_rmv_service(from, tmp_rmv_svc_);
    }
  }

  void register_socket(ctxid_pair_t ctxid_pr, aid_t const& skt)
  {
    if (ctxid_pr.second == socket_router)
    {
      pri_register_socket(router_list_, curr_router_list_, ctxid_pr.first, skt);
    }
    else if (ctxid_pr.second == socket_joint)
    {
      pri_register_socket(joint_list_, curr_joint_list_, ctxid_pr.first, skt);
    }
    else
    {
      pri_register_socket(conn_list_, curr_socket_list_, ctxid_pr.first, skt);
    }
  }

  aid_t select_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0)
  {
    aid_t skt = select_straight_socket(ctxid, target);
    if (skt == aid_nil)
    {
      skt = select_router(target);
    }
    return skt;
  }

  aid_t select_straight_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0)
  {
    aid_t skt;
    skt_list_t* skt_list = 0;
    skt_list_t::iterator* curr_skt = 0;

    if (ctxid != ctxid_nil)
    {
      typename conn_list_t::iterator itr(conn_list_.find(ctxid));
      if (itr != conn_list_.end())
      {
        skt_list = &itr->second.skt_list_;
        curr_skt = &itr->second.curr_skt_;
      }
    }
    else
    {
      if (!conn_list_.empty())
      {
        if (curr_socket_list_ != conn_list_.end())
        {
          skt_list = &curr_socket_list_->second.skt_list_;
          curr_skt = &curr_socket_list_->second.curr_skt_;
          if (target)
          {
            *target = curr_socket_list_->first;
          }
        }

        ++curr_socket_list_;
        if (curr_socket_list_ == conn_list_.end())
        {
          curr_socket_list_ = conn_list_.begin();
        }
      }
    }

    if (skt_list && !skt_list->empty())
    {
      GCE_ASSERT(curr_skt);
      skt_list_t::iterator& itr = *curr_skt;
      if (itr != skt_list->end())
      {
        skt = *itr;
      }
      ++itr;
      if (itr == skt_list->end())
      {
        itr = skt_list->begin();
      }
    }

    return skt;
  }

  aid_t select_joint_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0)
  {
    aid_t skt;
    skt_list_t* skt_list = 0;
    skt_list_t::iterator* curr_skt = 0;

    if (ctxid != ctxid_nil)
    {
      typename conn_list_t::iterator itr(joint_list_.find(ctxid));
      if (itr != joint_list_.end())
      {
        skt_list = &itr->second.skt_list_;
        curr_skt = &itr->second.curr_skt_;
      }
    }
    else
    {
      if (!joint_list_.empty())
      {
        if (curr_joint_list_ != joint_list_.end())
        {
          skt_list = &curr_joint_list_->second.skt_list_;
          curr_skt = &curr_joint_list_->second.curr_skt_;
          if (target)
          {
            *target = curr_joint_list_->first;
          }
        }

        ++curr_joint_list_;
        if (curr_joint_list_ == joint_list_.end())
        {
          curr_joint_list_ = joint_list_.begin();
        }
      }
    }

    if (skt_list && !skt_list->empty())
    {
      GCE_ASSERT(curr_skt);
      skt_list_t::iterator& itr = *curr_skt;
      if (itr != skt_list->end())
      {
        skt = *itr;
      }
      ++itr;
      if (itr == skt_list->end())
      {
        itr = skt_list->begin();
      }
    }

    return skt;
  }

  aid_t select_router(ctxid_t* target = 0)
  {
    aid_t skt;
    if (!router_list_.empty())
    {
      if (curr_router_list_ != router_list_.end())
      {
        skt_list_t& skt_list = curr_router_list_->second.skt_list_;
        skt_list_t::iterator& curr_skt = curr_router_list_->second.curr_skt_;
        if (target)
        {
          *target = curr_router_list_->first;
        }

        if (!skt_list.empty())
        {
          if (curr_skt != skt_list.end())
          {
            skt = *curr_skt;
          }
          ++curr_skt;
          if (curr_skt == skt_list.end())
          {
            curr_skt = skt_list.begin();
          }
        }
      }

      ++curr_router_list_;
      if (curr_router_list_ == router_list_.end())
      {
        curr_router_list_ = router_list_.begin();
      }
    }
    return skt;
  }

  void deregister_socket(ctxid_pair_t ctxid_pr, aid_t const& skt)
  {
    if (ctxid_pr.second == socket_router)
    {
      pri_deregister_socket(router_list_, ctxid_pr.first, skt);
    }
    else if (ctxid_pr.second == socket_joint)
    {
      pri_deregister_socket(joint_list_, ctxid_pr.first, skt);
    }
    else
    {
      pri_deregister_socket(conn_list_, ctxid_pr.first, skt);
    }
  }

  void send_already_exited(aid_t recver, aid_t const& sender)
  {
    aid_t target = filter_aid(recver);
    if (target != aid_nil)
    {
      message m(exit);
      std::string exit_msg("already exited");
      m << exit_already << exit_msg;

      pack& pk = alloc_pack(target);
      pk.tag_ = sender;
      pk.recver_ = recver;
      pk.skt_ = target;
      pk.msg_ = m;
      pk.is_err_ret_ = true;

      send(target, pk);
    }
  }

  void send_already_exited(aid_t const& recver, resp_t const& res)
  {
    aid_t target = filter_aid(recver);
    if (target != aid_nil)
    {
      message m(exit);
      std::string exit_msg("already exited");
      m << exit_already << exit_msg;

      pack& pk = alloc_pack(target);
      pk.tag_ = res;
      pk.recver_ = recver;
      pk.skt_ = target;
      pk.msg_ = m;
      pk.is_err_ret_ = true;

      send(target, pk);
    }
  }

  aid_t filter_aid(aid_t const& src)
  {
    aid_t target = aid_nil;

    bool is_local = check_local(src, ctxid_);
    if (is_local && check_local_valid(src, ctxid_, timestamp_))
    {
      target = src;
    }
    else
    {
      if (!is_local)
      {
        target = select_socket(src.ctxid_);
      }
    }
    return target;
  }

  aid_t filter_svcid(svcid_t const& src)
  {
    aid_t target = aid_nil;

    if (src.ctxid_ == ctxid_nil || src.ctxid_ == ctxid_)
    {
      target = find_service(src.name_);
    }
    else
    {
      target = select_socket(src.ctxid_);
    }
    return target;
  }

  svcid_t filter_svcid(match_t name)
  {
    svcid_t svcid = svcid_nil;

    typename global_service_list_t::iterator itr(global_service_list_.find(name));
    if (itr != global_service_list_.end())
    {
      ctxid_list& cl = itr->second;
      if (cl.curr_ctx_ != cl.ctx_list_.end())
      {
        ctxid_t ctxid = *cl.curr_ctx_;
        svcid = make_svcid(ctxid, name);
      }

      ++cl.curr_ctx_;
      if (cl.curr_ctx_ == cl.ctx_list_.end())
      {
        cl.curr_ctx_ = cl.ctx_list_.begin();
      }
    }
    return svcid;
  }

  virtual void stop()
  {
    stopped_ = true;
  }

  bool stopped() const
  {
    return stopped_;
  }

protected:
  void send_already_exit(pack& pk)
  {
    if (!pk.is_err_ret_)
    {
      if (link_t* link = boost::get<link_t>(&pk.tag_))
      {
        /// send actor exit msg
        send_already_exited(link->get_aid(), pk.recver_);
      }
      else if (request_t* req = boost::get<request_t>(&pk.tag_))
      {
        /// reply actor exit msg
        resp_t res(req->get_id(), pk.recver_);
        send_already_exited(req->get_aid(), res);
      }
    }
  }

private:
  typedef std::set<aid_t> skt_list_t;
  struct socket_list
  {
    skt_list_t skt_list_;
    skt_list_t::iterator curr_skt_;
  };
  typedef std::map<ctxid_t, socket_list> conn_list_t;

  void broadcast2socket(conn_list_t const& conn_list, aid_t const& from, message const& m)
  {
    BOOST_FOREACH(typename conn_list_t::value_type const& pr, conn_list)
    {
      skt_list_t const& skt_list = pr.second.skt_list_;
      if (!skt_list.empty())
      {
        send2skt(from, *skt_list.begin(), m);
      }
    }
  }

  void pri_register_socket(conn_list_t& conn_list, typename conn_list_t::iterator& curr, ctxid_t ctxid, aid_t const& skt)
  {
    std::pair<typename conn_list_t::iterator, bool> pr =
      conn_list.insert(std::make_pair(ctxid, skt_dummy_));
    skt_list_t& skt_list = pr.first->second.skt_list_;
    skt_list.insert(skt);
    if (skt_list.size() == 1)
    {
      pr.first->second.curr_skt_ = skt_list.begin();
    }

    if (conn_list.size() == 1)
    {
      curr = conn_list.begin();
    }
  }

  static void pri_deregister_socket(conn_list_t& conn_list, ctxid_t ctxid, aid_t const& skt)
  {
    typename conn_list_t::iterator itr(conn_list.find(ctxid));
    if (itr != conn_list.end())
    {
      socket_list& skt_list = itr->second;
      skt_list_t::iterator skt_itr = skt_list.skt_list_.find(skt);
      skt_list_t::iterator next_itr(skt_itr);
      bool is_curr = skt_itr == skt_list.curr_skt_;
      ++next_itr;
      skt_list.skt_list_.erase(skt_itr);
      if (is_curr)
      {
        skt_list.curr_skt_ = next_itr;
      }

      if (skt_list.skt_list_.empty())
      {
        conn_list.erase(itr);
      }
    }
  }

protected:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(context_t&, ctx_)
  GCE_CACHE_ALIGNED_VAR(size_t, index_)
  GCE_CACHE_ALIGNED_VAR(actor_type, type_)
  GCE_CACHE_ALIGNED_VAR(strand_t&, snd_)

  /// coro local vars
  ctxid_t const ctxid_;
  timestamp_t const timestamp_;

private:
  conn_list_t conn_list_;
  conn_list_t joint_list_;
  conn_list_t router_list_;
  typename conn_list_t::iterator curr_router_list_;
  typename conn_list_t::iterator curr_socket_list_;
  typename conn_list_t::iterator curr_joint_list_;
  socket_list const skt_dummy_;

  typedef std::map<match_t, aid_t> service_list_t;
  service_list_t service_list_;

  /// global service by name
  typedef std::set<ctxid_t> ctx_list_t;
  struct ctxid_list
  {
    ctx_list_t ctx_list_;
    ctx_list_t::iterator curr_ctx_;
  };
  typedef std::map<match_t, ctxid_list> global_service_list_t;
  global_service_list_t global_service_list_;
  ctxid_list const ctx_dummy_;
  adl::detail::ctxid_list const adl_ctx_dummy_;

  /// global service by ctxid
  typedef std::map<match_t, ctx_list_t*> global_svcitr_list_t;
  typedef std::map<ctxid_t, global_svcitr_list_t> global_ctxid_list_t;
  global_ctxid_list_t global_ctxid_list_;
  global_svcitr_list_t svcitr_dummy_;

  adl::detail::add_svc tmp_add_svc_;
  adl::detail::rmv_svc tmp_rmv_svc_;

  bool stopped_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_BASIC_SERVICE_HPP
