///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_CONTEXT_HPP
#define GCE_ACTOR_CONTEXT_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/thread_based_actor.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/optional.hpp>
#include <boost/container/vector.hpp>
#include <boost/lockfree/queue.hpp>
#include <vector>

namespace gce
{
typedef std::size_t thrid_t;
typedef boost::function<void (thrid_t)> thread_callback_t;
struct attributes
{
  attributes()
    : ios_(0)
    , id_(ctxid_nil)
    , thread_num_(boost::thread::hardware_concurrency())
    , per_thread_cache_pool_num_(1)
    , slice_num_(1)
    , actor_pool_reserve_size_(8)
    , socket_pool_reserve_size_(8)
    , acceptor_pool_reserve_size_(8)
    , max_cache_match_size_(32)
  {
  }

  io_service_t* ios_;
  ctxid_t id_;
  std::size_t thread_num_;
  std::size_t per_thread_cache_pool_num_;
  std::size_t slice_num_;
  std::size_t actor_pool_reserve_size_;
  std::size_t socket_pool_reserve_size_;
  std::size_t acceptor_pool_reserve_size_;
  std::size_t max_cache_match_size_;
  std::vector<thread_callback_t> thread_begin_cb_list_;
  std::vector<thread_callback_t> thread_end_cb_list_;
};

namespace detail
{
class cache_pool;
}

class slice;
class context
{
public:
  explicit context(attributes attrs = attributes());
  ~context();

public:
  inline io_service_t& get_io_service()
  {
    BOOST_ASSERT(ios_);
    return *ios_;
  }
  
  inline aid_t get_aid() const
  {
    return base_->get_aid();
  }

  inline void send(aid_t recver, message const& m)
  {
    base_->send(recver, m);
  }

  inline void send(svcid_t recver, message const& m)
  {
    base_->send(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    base_->relay(des, m);
  }

  inline void relay(svcid_t des, message& m)
  {
    base_->relay(des, m);
  }

  inline response_t request(aid_t recver, message const& m)
  {
    return base_->request(recver, m);
  }

  inline response_t request(svcid_t recver, message const& m)
  {
    return base_->request(recver, m);
  }

  inline void reply(aid_t recver, message const& m)
  {
    base_->reply(recver, m);
  }

  inline void link(aid_t target)
  {
    base_->link(target);
  }

  inline void monitor(aid_t target)
  {
    base_->monitor(target);
  }

  inline aid_t recv(message& msg, match const& mach = match())
  {
    return base_->recv(msg, mach);
  }

  inline aid_t recv(
    response_t res, message& msg, 
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    )
  {
    return base_->recv(res, msg, tmo);
  }

  inline void wait(duration_t dur)
  {
    base_->wait(dur);
  }

public:
  /// internal use
  inline attributes const& get_attributes() const { return attrs_; }
  inline timestamp_t get_timestamp() const { return timestamp_; }
  inline std::size_t get_cache_queue_size() const { return cache_queue_size_; }

  actor_t make_mixin();
  detail::cache_pool* select_cache_pool();
  slice& make_slice();

  void register_service(match_t name, aid_t svc, std::size_t cache_queue_index);
  void deregister_service(match_t name, aid_t svc, std::size_t cache_queue_index);

  void register_socket(ctxid_pair_t, aid_t skt, std::size_t cache_queue_index);
  void deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt, std::size_t cache_queue_index);

  inline detail::cache_pool* get_cache_pool()
  {
    return base_->get_cache_pool();
  }

  inline sid_t spawn(match_t func, match_t ctxid, std::size_t stack_size)
  {
    return base_->spawn(func, ctxid, stack_size);
  }

  inline void add_slice(slice& s)
  {
    base_->add_slice(s);
  }

  inline std::vector<slice*>& get_slice_list()
  {
    return base_->get_slice_list(); 
  }

  inline context* get_context()
  {
    return this;
  }

private:
  void run(
    thrid_t,
    std::vector<thread_callback_t> const&,
    std::vector<thread_callback_t> const&
    );
  void stop();

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(attributes, attrs_)
  GCE_CACHE_ALIGNED_VAR(timestamp_t const, timestamp_)

  /// select cache pool
  GCE_CACHE_ALIGNED_VAR(std::size_t, curr_cache_pool_)
  GCE_CACHE_ALIGNED_VAR(std::size_t, cache_pool_size_)
  GCE_CACHE_ALIGNED_VAR(std::size_t, cache_queue_size_)

  GCE_CACHE_ALIGNED_VAR(detail::unique_ptr<io_service_t>, ios_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<io_service_t::work>, work_)

  GCE_CACHE_ALIGNED_VAR(boost::thread_group, thread_group_)
  GCE_CACHE_ALIGNED_VAR(std::vector<detail::cache_pool*>, cache_pool_list_)
  
  GCE_CACHE_ALIGNED_VAR(std::vector<slice*>, slice_list_)
  GCE_CACHE_ALIGNED_VAR(boost::atomic_size_t, curr_slice_)

  GCE_CACHE_ALIGNED_VAR(boost::lockfree::queue<thread_based_actor*>, mixin_list_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<thread_based_actor>, base_)
};
}

#endif /// GCE_ACTOR_CONTEXT_HPP
