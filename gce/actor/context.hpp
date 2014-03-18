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
    , mixin_num_(1)
    , per_thread_cache_(1)
    , actor_pool_reserve_size_(8)
    , slice_pool_reserve_size_(8)
    , socket_pool_reserve_size_(8)
    , acceptor_pool_reserve_size_(8)
    , pack_pool_reserve_size_(8)
    , pack_pool_free_size_(8)
    , max_cache_match_size_(32)
    , gc_period_(1000)
  {
  }

  io_service_t* ios_;
  ctxid_t id_;
  std::size_t thread_num_;
  std::size_t mixin_num_;
  std::size_t per_thread_cache_;
  std::size_t actor_pool_reserve_size_;
  std::size_t slice_pool_reserve_size_;
  std::size_t socket_pool_reserve_size_;
  std::size_t acceptor_pool_reserve_size_;
  std::size_t pack_pool_reserve_size_;
  std::size_t pack_pool_free_size_;
  std::size_t max_cache_match_size_;
  boost::chrono::milliseconds gc_period_;
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
  detail::cache_pool* select_cache_pool(std::size_t i = size_nil);

  void register_service(match_t name, aid_t svc, detail::cache_pool*);
  void deregister_service(match_t name, aid_t svc, detail::cache_pool*);

  void register_socket(ctxid_pair_t, aid_t skt, detail::cache_pool*);
  void deregister_socket(ctxid_pair_t ctxid_pr, aid_t skt, detail::cache_pool*);

private:
  void run(
    thrid_t,
    std::vector<thread_callback_t> const&,
    std::vector<thread_callback_t> const&
    );
  void stop();
  void stop_mixin(detail::cache_pool*);
  void start_gc_timer(detail::cache_pool*);
  void gc(detail::cache_pool*, errcode_t const&);

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  GCE_CACHE_ALIGNED_VAR(attributes, attrs_)
  GCE_CACHE_ALIGNED_VAR(timestamp_t const, timestamp_)

  /// select cache pool
  GCE_CACHE_ALIGNED_VAR(std::size_t, curr_cache_pool_)
  GCE_CACHE_ALIGNED_VAR(std::size_t, cache_pool_size_)

  GCE_CACHE_ALIGNED_VAR(detail::unique_ptr<io_service_t>, ios_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<io_service_t::work>, work_)

  GCE_CACHE_ALIGNED_VAR(boost::thread_group, thread_group_)
  GCE_CACHE_ALIGNED_VAR(std::vector<detail::cache_pool*>, cache_pool_list_)

  GCE_CACHE_ALIGNED_VAR(std::vector<mixin*>, mixin_list_)
  GCE_CACHE_ALIGNED_VAR(boost::atomic_size_t, curr_mixin_)
};
}

#endif /// GCE_ACTOR_CONTEXT_HPP
