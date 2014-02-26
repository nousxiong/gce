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
    , thread_num_(boost::thread::hardware_concurrency())
    , mixin_num_(1)
    , per_thread_cache_(3)
    , per_mixin_cache_(2)
    , actor_pool_reserve_size_(8)
    , pack_pool_reserve_size_(8)
    , pack_pool_free_size_(8)
    , slice_pool_reserve_size_(8)
    , slice_pool_free_size_(8)
    , socket_pool_reserve_size_(8)
    , socket_pool_free_size_(8)
    , acceptor_pool_reserve_size_(8)
    , acceptor_pool_free_size_(8)
    , max_cache_match_size_(32)
    , gc_period_(1000)
  {
  }

  io_service_t* ios_;
  std::size_t thread_num_;
  std::size_t mixin_num_;
  std::size_t per_thread_cache_;
  std::size_t per_mixin_cache_;
  std::size_t actor_pool_reserve_size_;
  std::size_t pack_pool_reserve_size_;
  std::size_t pack_pool_free_size_;
  std::size_t slice_pool_reserve_size_;
  std::size_t slice_pool_free_size_;
  std::size_t socket_pool_reserve_size_;
  std::size_t socket_pool_free_size_;
  std::size_t acceptor_pool_reserve_size_;
  std::size_t acceptor_pool_free_size_;
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
  inline attributes const& get_attributes() const { return attrs_; }
  inline io_service_t& get_io_service() { return *ios_; }
  detail::cache_pool* select_cache_pool(std::size_t i = size_nil);
  mixin& make_mixin();

private:
  void run(
    thrid_t,
    std::vector<thread_callback_t> const&,
    std::vector<thread_callback_t> const&
    );
  void stop();
  void start_gc_timer(detail::cache_pool*);
  void gc(detail::cache_pool*, errcode_t const&);

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  GCE_CACHE_ALIGNED_VAR(attributes, attrs_)

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
