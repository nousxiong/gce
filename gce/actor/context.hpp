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
#include <gce/detail/cache_aligned_ptr.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/static_assert.hpp>
#include <boost/atomic.hpp>
#include <vector>

namespace gce
{
typedef std::size_t thrid_t;
typedef boost::function<void (thrid_t)> thread_callback_t;
struct attributes
{
  attributes()
    : thread_num_(boost::thread::hardware_concurrency())
    , mixin_num_(1)
    , per_thread_cache_(3)
    , per_mixin_cache_(2)
    , stack_scale_(stack_default)
    , actor_pool_reserve_size_(8)
    , thin_pool_reserve_size_(8)
    , timer_pool_reserve_size_(8)
    , timer_pool_free_size_(8)
    , pack_pool_reserve_size_(8)
    , pack_pool_free_size_(8)
    , slice_pool_reserve_size_(8)
    , slice_pool_free_size_(8)
    , socket_pool_reserve_size_(8)
    , socket_pool_free_size_(8)
    , acceptor_pool_reserve_size_(8)
    , acceptor_pool_free_size_(8)
    , max_cache_match_size_(16)
    , gc_period_(boost::chrono::milliseconds(1000))
  {
  }

  std::size_t thread_num_;
  std::size_t mixin_num_;
  std::size_t per_thread_cache_;
  std::size_t per_mixin_cache_;
  stack_scale_type stack_scale_;
  std::size_t actor_pool_reserve_size_;
  std::size_t thin_pool_reserve_size_;
  std::size_t timer_pool_reserve_size_;
  std::size_t timer_pool_free_size_;
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
  inline io_service_t* get_io_service() { return ios_.get(); }
  detail::cache_pool* select_cache_pool();
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

  BOOST_STATIC_ASSERT((GCE_CACHE_LINE_SIZE * 4 >= sizeof(attributes)));
  attributes attrs_;
  byte_t pad1_[GCE_CACHE_LINE_SIZE * 4 - sizeof(attributes)];

  std::size_t curr_cache_pool_; /// 使用轮询方式select cache pool
  byte_t pad2_[GCE_CACHE_LINE_SIZE - sizeof(std::size_t)];

  std::size_t cache_pool_size_;
  byte_t pad3_[GCE_CACHE_LINE_SIZE - sizeof(std::size_t)];

  typedef detail::unique_ptr<boost::asio::io_service> io_service_ptr;
  detail::cache_aligned_ptr<boost::asio::io_service, io_service_ptr> ios_;

  typedef detail::unique_ptr<boost::asio::io_service::work> work_ptr;
  detail::cache_aligned_ptr<boost::asio::io_service::work, work_ptr> work_;

  typedef detail::unique_ptr<boost::thread_group> thread_group_ptr;
  detail::cache_aligned_ptr<boost::thread_group, thread_group_ptr> thread_group_;

  typedef detail::cache_aligned_ptr<detail::cache_pool, detail::cache_pool*> cache_pool_ptr;
  typedef std::vector<cache_pool_ptr> cache_pool_list_t;
  typedef detail::unique_ptr<cache_pool_list_t> cache_pool_list_ptr;
  detail::cache_aligned_ptr<cache_pool_list_t, cache_pool_list_ptr> cache_pool_list_;

  typedef detail::cache_aligned_ptr<mixin, mixin*> mixin_ptr;
  typedef std::vector<mixin_ptr> mixin_list_t;
  typedef detail::unique_ptr<mixin_list_t> mixin_list_ptr;
  detail::cache_aligned_ptr<mixin_list_t, mixin_list_ptr> mixin_list_;

  boost::atomic_size_t curr_mixin_;
  byte_t pad4_[GCE_CACHE_LINE_SIZE - sizeof(boost::atomic_size_t)];
};
}

#endif /// GCE_ACTOR_CONTEXT_HPP
