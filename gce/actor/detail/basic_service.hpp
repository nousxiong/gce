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
#include <gce/actor/detail/msg_pool.hpp>
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
  virtual void on_recv_forth(self_t& sender_svc, send_pair& sp, msg_pool_t::line&) {};
  virtual void on_recv_back(self_t& sender_svc, send_pair& sp, msg_pool_t::line&) {};
  virtual void on_recv(pack&) {};
  virtual void on_attach(msg_pool_t::line&) {};
  virtual void handle_tick(pack&) {};

  virtual pack& alloc_pack(aid_t const& target) = 0;

  void send(aid_t const& recver, pack& pk)
  {
    GCE_ASSERT(recver.type_ != (byte_t)actor_addon);
    bool already_exit = true;
    if (pk.expiry_)
    {
      already_exit = true;
    }
    else
    {
      pk.concurrency_index_ = index_;
      pk.type_ = type_;
      if (in_pool(recver))
      {
        actor_index const& ai = pk.ai_;
        already_exit = false;
        pk.sid_ = recver.sid_;

        if (is_inpool() && ai.svc_id_ == index_)
        {
          ctx_.push_tick(index_, pk);
        }
        else
        {
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
    }

    if (already_exit)
    {
      /// no need worry about pk dealloc, bcz at this case, pk is pk_ member, not alloced
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
  virtual void pri_send(actor_index const&, pack&) = 0;

  virtual bool is_inpool() const
  {
    return false;
  }

  void send2skt(aid_t const& from, aid_t const& to, message const& m)
  {
    GCE_ASSERT(to.type_ == (byte_t)actor_socket);
    pack& pk = alloc_pack(to);
    pk.tag_ = from;
    pk.recver_ = to;
    pk.skt_ = to;
    //pk.msg_ = m;
    pk.setmsg(m);

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
    aid_t svc = aid_nil;
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

  bool has_socket(ctxid_t ctxid) const
  {
    return pri_has_socket(conn_list_, ctxid);
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
    else if (ctxid_pr.second == socket_conn)
    {
      pri_register_socket(conn_list_, curr_conn_list_, ctxid_pr.first, skt);
    }
    else
    {
      pri_register_socket(bind_list_, curr_bind_list_, ctxid_pr.first, skt);
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
    aid_t skt = pri_select_socket(conn_list_, curr_conn_list_, ctxid, target);
    if (skt == aid_nil)
    {
      skt = pri_select_socket(bind_list_, curr_bind_list_, ctxid, target);
    }
    return skt;
  }

  aid_t select_joint_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0)
  {
    return pri_select_socket(joint_list_, curr_joint_list_, ctxid, target);
  }

  aid_t select_router(ctxid_t* target = 0)
  {
    aid_t skt = aid_nil;
    if (!router_list_.empty())
    {
      if (curr_router_list_ != router_list_.end())
      {
        skt_list_t& skt_list = curr_router_list_->second.skt_list_;
        skt_list_t& reconn_list = curr_router_list_->second.reconn_list_;
        skt_list_t::iterator& curr_skt = curr_router_list_->second.curr_skt_;
        skt_list_t::iterator& curr_reconn = curr_router_list_->second.curr_reconn_;
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
        else if (!reconn_list.empty())
        {
          if (curr_reconn != reconn_list.end())
          {
            skt = *curr_reconn;
          }
          ++curr_reconn;
          if (curr_reconn == reconn_list.end())
          {
            curr_reconn = reconn_list.begin();
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
    else if (ctxid_pr.second == socket_conn)
    {
      pri_deregister_socket(conn_list_, ctxid_pr.first, skt);
    }
    else
    {
      pri_deregister_socket(bind_list_, ctxid_pr.first, skt);
    }
  }

  void conn_socket(ctxid_pair_t ctxid_pr, aid_t const& skt)
  {
    if (ctxid_pr.second == socket_router)
    {
      pri_conn_socket(router_list_, ctxid_pr.first, skt);
    }
    else if (ctxid_pr.second == socket_joint)
    {
      pri_conn_socket(joint_list_, ctxid_pr.first, skt);
    }
    else if (ctxid_pr.second == socket_conn)
    {
      pri_conn_socket(conn_list_, ctxid_pr.first, skt);
    }
    else
    {
      pri_conn_socket(bind_list_, ctxid_pr.first, skt);
    }
  }

  void disconn_socket(ctxid_pair_t ctxid_pr, aid_t const& skt)
  {
    if (ctxid_pr.second == socket_router)
    {
      pri_disconn_socket(router_list_, ctxid_pr.first, skt);
    }
    else if (ctxid_pr.second == socket_joint)
    {
      pri_disconn_socket(joint_list_, ctxid_pr.first, skt);
    }
    else if (ctxid_pr.second == socket_conn)
    {
      pri_disconn_socket(conn_list_, ctxid_pr.first, skt);
    }
    else
    {
      pri_disconn_socket(bind_list_, ctxid_pr.first, skt);
    }
  }

  void send_already_exited(aid_t const& recver, aid_t const& sender)
  {
    aid_t target = filter_aid(recver);
    if (target != aid_nil)
    {
      pack& pk = alloc_pack(target);
      pk.tag_ = exit_t(exit_already, sender);
      pk.recver_ = recver;
      pk.skt_ = target;
      message& msg = pk.getmsg();
      msg.clear();
      msg.set_type(exit);
      msg << exit_already << "already exited";
      pk.is_err_ret_ = true;

      send(target, pk);
    }
  }

  void send_already_exited(aid_t const& recver, resp_t const& res)
  {
    aid_t target = filter_aid(recver);
    if (target != aid_nil)
    {
      pack& pk = alloc_pack(target);
      pk.tag_ = res;
      pk.recver_ = recver;
      pk.skt_ = target;
      message& msg = pk.getmsg();
      msg.clear();
      msg.set_type(exit);
      msg << exit_already << "already exited";
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
    skt_list_t reconn_list_;
    skt_list_t::iterator curr_reconn_;
  };
  typedef std::map<ctxid_t, socket_list> conn_list_t;
  
  bool pri_has_socket(conn_list_t const& conn_list, ctxid_t ctxid) const
  {
    typename conn_list_t::const_iterator itr = conn_list.find(ctxid);
    if (itr == conn_list.end())
    {
      return false;
    }
    return !itr->second.skt_list_.empty() || !itr->second.reconn_list_.empty();
  }

  static aid_t pri_select_socket(
    conn_list_t& conn_list, 
    typename conn_list_t::iterator& curr_socket_list, 
    ctxid_t ctxid, ctxid_t* target
    )
  {
    aid_t skt = aid_nil;
    skt_list_t* skt_list = 0;
    skt_list_t::iterator* curr_skt = 0;

    if (ctxid != ctxid_nil)
    {
      typename conn_list_t::iterator itr(conn_list.find(ctxid));
      if (itr != conn_list.end())
      {
        if (!itr->second.skt_list_.empty())
        {
          skt_list = &itr->second.skt_list_;
          curr_skt = &itr->second.curr_skt_;
        }
        else
        {
          skt_list = &itr->second.reconn_list_;
          curr_skt = &itr->second.curr_reconn_;
        }
      }
    }
    else
    {
      if (!conn_list.empty())
      {
        if (curr_socket_list != conn_list.end())
        {
          if (!curr_socket_list->second.skt_list_.empty())
          {
            skt_list = &curr_socket_list->second.skt_list_;
            curr_skt = &curr_socket_list->second.curr_skt_;
          }
          else
          {
            skt_list = &curr_socket_list->second.reconn_list_;
            curr_skt = &curr_socket_list->second.curr_reconn_;
          }
          if (target)
          {
            *target = curr_socket_list->first;
          }
        }

        ++curr_socket_list;
        if (curr_socket_list == conn_list.end())
        {
          curr_socket_list = conn_list.begin();
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
      //socket_list& skt_list = itr->second;
      skt_list_t& skt_list = itr->second.skt_list_;
      skt_list_t& reconn_list = itr->second.reconn_list_;
      skt_list_t::iterator& curr_skt = itr->second.curr_skt_;
      skt_list_t::iterator& curr_reconn = itr->second.curr_reconn_;

      skt_list_t::iterator skt_itr = skt_list.find(skt);
      if (skt_itr != skt_list.end())
      {
        erase_socket(skt_list, skt_itr, curr_skt);
      }
      else
      {
        skt_itr = reconn_list.find(skt);
        if (skt_itr != reconn_list.end())
        {
          erase_socket(reconn_list, skt_itr, curr_reconn);
        }
      }

      if (skt_list.empty() && reconn_list.empty())
      {
        conn_list.erase(itr);
      }
    }
  }

  static void erase_socket(skt_list_t& skt_list, skt_list_t::iterator& itr, skt_list_t::iterator& curr)
  {
    skt_list_t::iterator next_itr(itr);
    ++next_itr;
    bool is_curr = itr == curr;
    skt_list.erase(itr);
    if (is_curr)
    {
      curr = next_itr;
    }
  }

  static void pri_conn_socket(conn_list_t& conn_list, ctxid_t ctxid, aid_t const& skt)
  {
    typename conn_list_t::iterator itr(conn_list.find(ctxid));
    if (itr != conn_list.end())
    {
      skt_list_t& skt_list = itr->second.skt_list_;
      skt_list_t& reconn_list = itr->second.reconn_list_;
      skt_list_t::iterator& curr_skt = itr->second.curr_skt_;
      skt_list_t::iterator& curr_reconn = itr->second.curr_reconn_;

      move_socket(reconn_list, curr_reconn, skt_list, curr_skt, skt);
    }
  }

  static void pri_disconn_socket(conn_list_t& conn_list, ctxid_t ctxid, aid_t const& skt)
  {
    typename conn_list_t::iterator itr(conn_list.find(ctxid));
    if (itr != conn_list.end())
    {
      skt_list_t& skt_list = itr->second.skt_list_;
      skt_list_t& reconn_list = itr->second.reconn_list_;
      skt_list_t::iterator& curr_skt = itr->second.curr_skt_;
      skt_list_t::iterator& curr_reconn = itr->second.curr_reconn_;

      move_socket(skt_list, curr_skt, reconn_list, curr_reconn, skt);
    }
  }

  static void move_socket(
    skt_list_t& from, 
    skt_list_t::iterator& curr_from,
    skt_list_t& to, 
    skt_list_t::iterator& curr_to,
    aid_t const& skt
    )
  {
    skt_list_t::iterator skt_itr = from.find(skt);
    if (skt_itr != from.end())
    {
      erase_socket(from, skt_itr, curr_from);
    }

    to.insert(skt);
    if (to.size() == 1)
    {
      curr_to = to.begin();
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
  conn_list_t bind_list_;
  conn_list_t joint_list_;
  conn_list_t router_list_;
  typename conn_list_t::iterator curr_conn_list_;
  typename conn_list_t::iterator curr_bind_list_;
  typename conn_list_t::iterator curr_router_list_;
  typename conn_list_t::iterator curr_joint_list_;
  socket_list const skt_dummy_;

  typedef std::map<match_t, aid_t> service_list_t;
  service_list_t service_list_;

  bool stopped_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_BASIC_SERVICE_HPP
