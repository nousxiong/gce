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

class nonblocking_actor;
class thread_mapped_actor;
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

public:
  /// internal use
  inline attributes const& get_attributes() const { return attrs_; }
  inline timestamp_t get_timestamp() const { return timestamp_; }
  inline std::size_t get_cache_queue_size() const { return cache_queue_size_; }

  thread_mapped_actor& make_thread_mapped_actor();
  detail::cache_pool* select_cache_pool();
  nonblocking_actor& make_nonblocking_actor();

  void register_service(match_t name, aid_t svc, std::size_t cache_queue_index);
  void deregister_service(match_t name, aid_t svc, std::size_t cache_queue_index);

  void register_socket(ctxid_pair_t, aid_t skt, std::size_t cache_queue_index);
  void deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt, std::size_t cache_queue_index);

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
  
  GCE_CACHE_ALIGNED_VAR(std::vector<nonblocking_actor*>, nonblocking_actor_list_)
  GCE_CACHE_ALIGNED_VAR(boost::atomic_size_t, curr_nonblocking_actor_)

  GCE_CACHE_ALIGNED_VAR(boost::lockfree::queue<thread_mapped_actor*>, thread_mapped_actor_list_)
};
}

#endif /// GCE_ACTOR_CONTEXT_HPP
