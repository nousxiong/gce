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
#include <boost/assert.hpp>
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
  basic_service(context_t& ctx, strand_t& snd, std::size_t index, actor_type type)
    : ctx_(ctx)
    , index_(index)
    , type_(type)
    , snd_(snd)
    , ctxid_(ctx_.get_ctxid())
    , timestamp_(ctx_.get_timestamp())
  {
  }

public:
  virtual void on_recv_forth(self_t& sender_svc, send_pair& sp) {};
  virtual void on_recv_back(self_t& sender_svc, send_pair& sp) {};
  virtual void on_recv(pack&) {};

  virtual pack& alloc_pack(aid_t const& target) = 0;

  void send(aid_t const& recver, pack& pk)
  {
    pk.concurrency_index_ = index_;
    pk.type_ = type_;
    bool already_exit = true;
    if (recver.in_pool())
    {
      actor_index ai = recver.get_actor_index(ctxid_, timestamp_);
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
      listener* a = recver.get_actor_ptr(ctxid_, timestamp_);
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

protected:
  virtual void pri_send(actor_index, pack&) = 0;

public:
  context_t& get_context()
  {
    return ctx_;
  }

  std::size_t get_index() const
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
    service_list_.insert(std::make_pair(name, svc));
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

  void register_socket(ctxid_pair_t ctxid_pr, aid_t const& skt)
  {
    if (ctxid_pr.second == socket_router)
    {
      std::pair<typename conn_list_t::iterator, bool> pr =
        router_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
      skt_list_t& skt_list = pr.first->second.skt_list_;
      skt_list.insert(skt);
      if (skt_list.size() == 1)
      {
        pr.first->second.curr_skt_ = skt_list.begin();
      }

      if (router_list_.size() == 1)
      {
        curr_router_list_ = router_list_.begin();
      }
    }
    else if (ctxid_pr.second == socket_joint)
    {
      std::pair<typename conn_list_t::iterator, bool> pr =
        joint_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
      skt_list_t& skt_list = pr.first->second.skt_list_;
      skt_list.insert(skt);
      if (skt_list.size() == 1)
      {
        pr.first->second.curr_skt_ = skt_list.begin();
      }

      if (joint_list_.size() == 1)
      {
        curr_joint_list_ = joint_list_.begin();
      }
    }
    else
    {
      std::pair<typename conn_list_t::iterator, bool> pr =
        conn_list_.insert(std::make_pair(ctxid_pr.first, dummy_));
      skt_list_t& skt_list = pr.first->second.skt_list_;
      skt_list.insert(skt);
      if (skt_list.size() == 1)
      {
        pr.first->second.curr_skt_ = skt_list.begin();
      }

      if (conn_list_.size() == 1)
      {
        curr_socket_list_ = conn_list_.begin();
      }
    }
  }

  aid_t select_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0)
  {
    aid_t skt = select_straight_socket(ctxid, target);
    if (!skt)
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
      BOOST_ASSERT(curr_skt);
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
      BOOST_ASSERT(curr_skt);
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
      typename conn_list_t::iterator itr(router_list_.find(ctxid_pr.first));
      if (itr != router_list_.end())
      {
        socket_list& skt_list = itr->second;
        skt_list.skt_list_.erase(skt);
        skt_list.curr_skt_ = skt_list.skt_list_.begin();
      }
    }
    else if (ctxid_pr.second == socket_joint)
    {
      typename conn_list_t::iterator itr(joint_list_.find(ctxid_pr.first));
      if (itr != joint_list_.end())
      {
        socket_list& skt_list = itr->second;
        skt_list.skt_list_.erase(skt);
        skt_list.curr_skt_ = skt_list.skt_list_.begin();
      }
    }
    else
    {
      typename conn_list_t::iterator itr(conn_list_.find(ctxid_pr.first));
      if (itr != conn_list_.end())
      {
        socket_list& skt_list = itr->second;
        skt_list.skt_list_.erase(skt);
        skt_list.curr_skt_ = skt_list.skt_list_.begin();
      }
    }
  }

  void send_already_exited(aid_t recver, aid_t const& sender)
  {
    aid_t target = filter_aid(recver);
    if (target)
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
    if (target)
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
    aid_t target;

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
    aid_t target;

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

protected:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(context_t&, ctx_)
  GCE_CACHE_ALIGNED_VAR(std::size_t, index_)
  GCE_CACHE_ALIGNED_VAR(actor_type, type_)
  GCE_CACHE_ALIGNED_VAR(strand_t&, snd_)

  /// thread local vars
  ctxid_t const ctxid_;
  timestamp_t const timestamp_;

private:
  typedef std::set<aid_t> skt_list_t;
  struct socket_list
  {
    skt_list_t skt_list_;
    skt_list_t::iterator curr_skt_;
  };

  typedef std::map<ctxid_t, socket_list> conn_list_t;
  conn_list_t conn_list_;
  conn_list_t joint_list_;
  conn_list_t router_list_;
  typename conn_list_t::iterator curr_router_list_;
  typename conn_list_t::iterator curr_socket_list_;
  typename conn_list_t::iterator curr_joint_list_;
  socket_list dummy_;

  typedef std::map<match_t, aid_t> service_list_t;
  service_list_t service_list_;

  bool stopped_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_BASIC_SERVICE_HPP
