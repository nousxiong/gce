///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_NONBLOCKED_ACTOR_HPP
#define GCE_ACTOR_DETAIL_NONBLOCKED_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/attributes.hpp>
#include <gce/actor/detail/actor_service.hpp>
#include <gce/actor/detail/basic_actor.hpp>
#include <gce/detail/scope.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <deque>
#include <vector>
#include <utility>

namespace gce
{
namespace detail
{
template <typename Context>
class nonblocked_actor
  : public basic_actor<Context>
{
public:
  typedef Context context_t;

private:
  typedef basic_actor<context_t> base_t;
  typedef nonblocked_actor<context_t> self_t;
  typedef actor_service<self_t, false> service_t;

  struct cache_queue
  {
    cache_queue()
      : index_base_(0)
      , head_(0)
      , garbage_tail_(u64_nil)
    {
    }

    cache_queue(cache_queue const& other)
      : que_(other.que_)
      , index_base_(other.index_base_)
      , head_(other.head_)
      , garbage_tail_(
          other.garbage_tail_.load(boost::memory_order_relaxed)
          )
    {
    }

    cache_queue& operator=(cache_queue const& rhs)
    {
      if (this != &rhs)
      {
        que_ = rhs.que_;
        index_base_ = rhs.index_base_;
        head_ = rhs.head_;
        garbage_tail_.store(
          rhs.garbage_tail_.load(boost::memory_order_relaxed),
          boost::memory_order_relaxed
          );
      }
      return *this;
    }

    /// Ensure start from a new cache line.
    byte_t pad0_[GCE_CACHE_LINE_SIZE];

    GCE_CACHE_ALIGNED_VAR(std::deque<pack>, que_)
    GCE_CACHE_ALIGNED_VAR(uint64_t, index_base_)
    GCE_CACHE_ALIGNED_VAR(uint64_t, head_)
    GCE_CACHE_ALIGNED_VAR(boost::atomic<uint64_t>, garbage_tail_)
  };
  typedef std::vector<cache_queue> cache_queue_list_t;
  typedef boost::array<cache_queue_list_t, actor_num> cache_queue_list_arr_t;
  typedef std::vector<pack*> pack_list_t;
  typedef boost::array<pack_list_t, actor_num> gc_t;

public:
  nonblocked_actor(context_t& ctx, service_t& svc, size_t index)
    : base_t(
      ctx, svc, actor_nonblocked,
      make_aid(ctx.get_ctxid(), ctx.get_timestamp(), this, sid_t(0))
      )
    , pack_queue_(1024)
    , svc_(svc)
  {
    BOOST_FOREACH(cache_queue_list_t& que_list, cache_queue_list_arr_)
    {
      que_list.resize(base_t::ctx_.get_concurrency_size());
    }
  }

  ~nonblocked_actor()
  {
  }

public:
  void send(aid_t const& recver, message const& m)
  {
    base_t::pri_send(recver, m);
  }

  void send(svcid_t const& recver, message const& m)
  {
    base_t::pri_send_svc(recver, m);
  }

  template <typename Recver>
  void send(Recver recver, message const& m)
  {
    base_t::pri_send_svcs(recver, m);
  }

  void relay(aid_t const& des, message& m)
  {
    base_t::pri_relay(des, m);
  }

  void relay(svcid_t const& des, message& m)
  {
    base_t::pri_relay_svc(des, m);
  }

  template <typename Recver>
  void relay(Recver des, message& m)
  {
    base_t::pri_relay_svcs(des, m);
  }

  resp_t request(aid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request(res, recver, m);
    return res;
  }

  resp_t request(svcid_t const& recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid(), recver);
    base_t::pri_request_svc(res, recver, m);
    return res;
  }

  template <typename Recver>
  resp_t request(Recver recver, message const& m)
  {
    resp_t res(base_t::new_request(), base_t::get_aid());
    base_t::pri_request_svcs(res, recver, m);
    return res;
  }

  void reply(aid_t const& recver, message const& m)
  {
    base_t::pri_reply(recver, m);
  }

  void link(aid_t const& target)
  {
    base_t::pri_link(target);
  }

  void monitor(aid_t const& target)
  {
    base_t::pri_monitor(target);
  }

  aid_t recv(message& msg, match_list_t const& match_list = match_list_t(), recver_t const& recver = recver_t())
  {
    aid_t sender;
    recv_t rcv;

    move_pack();
    if (!base_t::mb_.pop(rcv, msg, match_list, recver))
    {
      return sender;
    }

    if (aid_t* aid = boost::get<aid_t>(&rcv))
    {
      sender = *aid;
      msg.set_relay(*aid);
    }
    else if (request_t* req = boost::get<request_t>(&rcv))
    {
      sender = req->get_aid();
      msg.set_relay(*req);
    }
    else if (exit_t* ex = boost::get<exit_t>(&rcv))
    {
      sender = ex->get_aid();
    }

    return sender;
  }

  aid_t respond(resp_t res, message& msg)
  {
    aid_t sender;

    move_pack();
    if (!base_t::mb_.pop(res, msg))
    {
      return sender;
    }

    sender = res.get_aid();
    return sender;
  }

public:
  /// internal use
  typedef gce::nonblocked type;

  static actor_type get_type()
  {
    return actor_nonblocked;
  }

  service_t& get_service()
  {
    return svc_;
  }

  void on_recv(pack& pk)
  {
    GCE_ASSERT(pk.concurrency_index_ != size_nil);
    GCE_ASSERT(pk.type_ != actor_nil);

    cache_queue& cac_que = cache_queue_list_arr_[pk.type_][pk.concurrency_index_];
    uint64_t garbage_tail =
      cac_que.garbage_tail_.load(boost::memory_order_relaxed);

    if (garbage_tail != u64_nil && garbage_tail >= cac_que.head_)
    {
      size_t size = garbage_tail - cac_que.head_ + 1;
      GCE_ASSERT(cac_que.que_.size() >= size)(cac_que.que_.size())(size);
      for (size_t i=0; i<size; ++i)
      {
        cac_que.que_.pop_front();
      }

      if (cac_que.que_.empty())
      {
        cac_que.head_ = cac_que.index_base_;
      }
      else
      {
        cac_que.head_ = cac_que.que_.front().cache_index_;
      }
    }

    pk.cache_index_ = cac_que.index_base_++;
    cac_que.que_.push_back(pk);
    pack_queue_.push(&cac_que.que_.back());
  }

  void on_addon_recv(pack& pk)
  {
    on_recv(pk);
  }

  void register_service(match_t name, aid_t const& svc, actor_type type, size_t concurrency_index)
  {
    pack pk = make_pack(type, concurrency_index);
    message m(msg_reg_svc);
    m << name << svc;
    pk.msg_ = m;
    on_recv(pk);
  }

  void deregister_service(match_t name, aid_t const& svc, actor_type type, size_t concurrency_index)
  {
    pack pk = make_pack(type, concurrency_index);
    message m(msg_dereg_svc);
    m << name << svc;
    pk.msg_ = m;
    on_recv(pk);
  }

  void add_service(match_t name, ctxid_t ctxid, actor_type type, size_t concurrency_index)
  {
    pack pk = make_pack(type, concurrency_index);
    message m(msg_add_svc);
    m << name << ctxid;
    pk.msg_ = m;
    on_recv(pk);
  }

  void rmv_service(match_t name, ctxid_t ctxid, actor_type type, size_t concurrency_index)
  {
    pack pk = make_pack(type, concurrency_index);
    message m(msg_rmv_svc);
    m << name << ctxid;
    pk.msg_ = m;
    on_recv(pk);
  }

  void register_socket(ctxid_pair_t ctxid_pr, aid_t const& skt, actor_type type, size_t concurrency_index)
  {
    pack pk = make_pack(type, concurrency_index);
    message m(msg_reg_skt);
    m << ctxid_pr << skt;
    pk.msg_ = m;
    on_recv(pk);
  }

  void deregister_socket(ctxid_pair_t ctxid_pr, aid_t const& skt, actor_type type, size_t concurrency_index)
  {
    pack pk = make_pack(type, concurrency_index);
    message m(msg_dereg_skt);
    m << ctxid_pr << skt;
    pk.msg_ = m;
    on_recv(pk);
  }

private:
  void release_pack()
  {
    for (int ty=0; ty<actor_num; ++ty)
    {
      pack_list_t& pack_list = gc_[ty];
      for (size_t i=0, size=pack_list.size(); i<size; ++i)
      {
        pack* pk = pack_list[i];
        if (pk)
        {
          cache_queue_list_arr_[ty][i].garbage_tail_ = pk->cache_index_;
        }
      }
    }
  }

  void move_pack()
  {
    BOOST_FOREACH(pack_list_t& pack_list, gc_)
    {
      pack_list.clear();
      pack_list.resize(base_t::ctx_.get_concurrency_size(), 0);
    }

    scope scp(boost::bind(&nonblocked_actor::release_pack, this));
    pack* pk = 0;
    while (pack_queue_.pop(pk))
    {
      gc_[pk->type_][pk->concurrency_index_] = pk;
      handle_recv(*pk);
    }
  }

  pack make_pack(actor_type type, size_t concurrency_index)
  {
    pack pk;
    pk.concurrency_index_ = concurrency_index;
    pk.type_ = type;
    aid_t self = base_t::get_aid();
    pk.tag_ = self;
    pk.recver_ = self;
    return pk;
  }

  void handle_recv(pack& pk)
  {
    bool is_response = false;
    if (aid_t* aid = boost::get<aid_t>(&pk.tag_))
    {
      match_t type = pk.msg_.get_type();
      if (type == msg_reg_skt)
      {
        ctxid_pair_t ctxid_pr;
        aid_t skt;
        pk.msg_ >> ctxid_pr >> skt;
        base_t::basic_svc_.register_socket(ctxid_pr, skt);
      }
      else if (type == msg_dereg_skt)
      {
        ctxid_pair_t ctxid_pr;
        aid_t skt;
        pk.msg_ >> ctxid_pr >> skt;
        base_t::basic_svc_.deregister_socket(ctxid_pr, skt);
      }
      else if (type == msg_reg_svc)
      {
        match_t name;
        aid_t svc;
        pk.msg_ >> name >> svc;
        base_t::basic_svc_.register_service(name, svc);
      }
      else if (type == msg_dereg_svc)
      {
        match_t name;
        aid_t svc;
        pk.msg_ >> name >> svc;
        base_t::basic_svc_.deregister_service(name, svc);
      }
      else if (type == msg_add_svc)
      {
        match_t name;
        ctxid_t ctxid;
        pk.msg_ >> name >> ctxid;
        base_t::basic_svc_.add_service(name, ctxid);
      }
      else if (type == msg_rmv_svc)
      {
        match_t name;
        ctxid_t ctxid;
        pk.msg_ >> name >> ctxid;
        base_t::basic_svc_.rmv_service(name, ctxid);
      }
      else
      {
        base_t::mb_.push(*aid, pk.msg_);
      }
    }
    else if (request_t* req = boost::get<request_t>(&pk.tag_))
    {
      base_t::mb_.push(*req, pk.msg_);
    }
    else if (link_t* link = boost::get<link_t>(&pk.tag_))
    {
      base_t::add_link(link->get_aid(), pk.skt_);
      return;
    }
    else if (exit_t* ex = boost::get<exit_t>(&pk.tag_))
    {
      base_t::mb_.push(*ex, pk.msg_);
      base_t::remove_link(ex->get_aid());
    }
    else if (resp_t* res = boost::get<resp_t>(&pk.tag_))
    {
      is_response = true;
      base_t::mb_.push(*res, pk.msg_);
    }
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(cache_queue_list_arr_t, cache_queue_list_arr_)
  GCE_CACHE_ALIGNED_VAR(boost::lockfree::queue<pack*>, pack_queue_)

  // local
  service_t& svc_;
  gc_t gc_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_NONBLOCKED_ACTOR_HPP
