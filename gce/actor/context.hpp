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
#include <gce/actor/thread.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <gce/detail/mpsc_queue.hpp>
#include <boost/thread/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/container/deque.hpp>
#include <boost/optional.hpp>
#include <vector>

namespace gce
{

struct attributes
{
  attributes()
    : ios_(0)
    , id_(ctxid_nil)
    , thread_num_(boost::thread::hardware_concurrency())
    , actor_pool_reserve_size_(8)
    , socket_pool_reserve_size_(8)
    , acceptor_pool_reserve_size_(8)
    , pack_pool_reserve_size_(8)
    , pack_pool_cache_size_(32)
    , max_cache_match_size_(32)
    , max_wait_counter_size_(10000)
  {
  }

  io_service_t* ios_;
  ctxid_t id_;
  std::size_t thread_num_;
  std::size_t actor_pool_reserve_size_;
  std::size_t socket_pool_reserve_size_;
  std::size_t acceptor_pool_reserve_size_;
  std::size_t pack_pool_reserve_size_;
  std::size_t pack_pool_cache_size_;
  std::size_t max_cache_match_size_;
  std::size_t max_wait_counter_size_;
  std::vector<thread_callback_t> thread_begin_cb_list_;
  std::vector<thread_callback_t> thread_end_cb_list_;
};

namespace detail
{
class cache_pool;
}

class mixin;
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

  mixin& make_mixin();

public:
  /// internal use
  inline attributes const& get_attributes() const { return attrs_; }
  inline timestamp_t get_timestamp() const { return timestamp_; }
  thread& select_thread();

  void register_service(match_t name, aid_t svc);
  void deregister_service(match_t name, aid_t svc);

  void register_socket(ctxid_pair_t, aid_t skt);
  void deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt);

private:
  void stop();

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(attributes, attrs_)
  GCE_CACHE_ALIGNED_VAR(timestamp_t const, timestamp_)

  /// select thread
  GCE_CACHE_ALIGNED_VAR(std::size_t, curr_thread_)
  GCE_CACHE_ALIGNED_VAR(std::size_t const, thread_size_)

  GCE_CACHE_ALIGNED_VAR(detail::unique_ptr<io_service_t>, ios_)

  GCE_CACHE_ALIGNED_VAR(boost::thread_group, thread_group_)
  GCE_CACHE_ALIGNED_VAR(detail::mpsc_queue<mixin>, mixin_list_)
  GCE_CACHE_ALIGNED_VAR(boost::container::deque<thread>, thread_list_)
};
}

#endif /// GCE_ACTOR_CONTEXT_HPP
