///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_NONBLOCKING_ACTOR_HPP
#define GCE_ACTOR_NONBLOCKING_ACTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <gce/actor/detail/pack.hpp>
#include <boost/lockfree/queue.hpp>
#include <deque>
#include <vector>
#include <utility>

namespace gce
{
class context;
struct attributes;

class nonblocking_actor
  : public basic_actor
{
  typedef basic_actor base_type;

public:
  nonblocking_actor(context& ctx, std::size_t index);
  ~nonblocking_actor();

public:
  inline void send(aid_t recver, message const& m)
  {
    base_type::pri_send(recver, m);
  }

  inline void send(svcid_t recver, message const& m)
  {
    base_type::pri_send_svc(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    base_type::pri_relay(des, m);
  }

  inline void relay(svcid_t des, message& m)
  {
    base_type::pri_relay_svc(des, m);
  }

  inline response_t request(aid_t recver, message const& m)
  {
    response_t res(base_type::new_request(), get_aid(), recver);
    base_type::pri_request(res, recver, m);
    return res;
  }

  inline response_t request(svcid_t recver, message const& m)
  {
    response_t res(base_type::new_request(), get_aid(), recver);
    base_type::pri_request_svc(res, recver, m);
    return res;
  }

  inline void reply(aid_t recver, message const& m)
  {
    base_type::pri_reply(recver, m);
  }

  inline void link(aid_t target)
  {
    base_type::pri_link(target);
  }

  inline void monitor(aid_t target)
  {
    base_type::pri_monitor(target);
  }

  aid_t recv(message&, match_list_t const& match_list = match_list_t());
  aid_t recv(response_t, message&);

public:
  /// internal use
  void on_recv(detail::pack&, detail::send_hint);

  void register_service(match_t name, aid_t svc, std::size_t cache_queue_index);
  void deregister_service(match_t name, aid_t svc, std::size_t cache_queue_index);

  void register_socket(ctxid_pair_t, aid_t skt, std::size_t cache_queue_index);
  void deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt, std::size_t cache_queue_index);

private:
  void release_pack();
  void move_pack();
  void handle_recv(detail::pack&);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

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

    GCE_CACHE_ALIGNED_VAR(std::deque<detail::pack>, que_)
    GCE_CACHE_ALIGNED_VAR(boost::uint64_t, index_base_)
    GCE_CACHE_ALIGNED_VAR(boost::uint64_t, head_)
    GCE_CACHE_ALIGNED_VAR(boost::atomic<boost::uint64_t>, garbage_tail_)
  };

  GCE_CACHE_ALIGNED_VAR(std::vector<cache_queue>, cache_queue_list_)
  GCE_CACHE_ALIGNED_VAR(boost::lockfree::queue<detail::pack*>, pack_queue_)

  // local
  detail::cache_pool cac_pool_;
  std::vector<detail::pack*> gc_;
};
}

#endif /// GCE_ACTOR_NONBLOCKING_ACTOR_HPP
