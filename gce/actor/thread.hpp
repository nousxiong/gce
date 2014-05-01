///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_THREAD_HPP
#define GCE_ACTOR_THREAD_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/thread_fwd.hpp>
#include <gce/actor/detail/cache_pool.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace gce
{
class context;
class mixin;
class thread
{
public:
  thread(context&, thrid_t);
  ~thread();

public:
  inline context* get_context() { return ctx_; }
  inline ctxid_t get_ctxid() const { return ctxid_; }
  inline ctxid_t get_timestamp() const { return timestamp_; }
  inline std::size_t get_max_cache_match_size() const { return max_cache_match_size_; }
  inline thrid_t get_thrid() const { return thrid_; }

public:
  /// internal use
  inline io_service_t& get_io_service() { return ios_; }
  inline detail::cache_pool& get_cache_pool() { return cac_pool_; }

  void run(
    std::vector<thread_callback_t> const&,
    std::vector<thread_callback_t> const&
    );
  void stop();

  inline bool stopped() const { return stopped_; }

  template <typename F>
  inline void post(thread* from, F f)
  {
    detail::pack* pk = from->get_cache_pool().get_pack();
    pk->f_ = f;
    BOOST_ASSERT(pk->f_);
    pack_queue_.push(pk);
  }

  template <typename F>
  inline void post(detail::pack* pk, F f)
  {
    pk->f_ = f;
    BOOST_ASSERT(pk->f_);
    pack_queue_.push(pk);
  }

  inline void post(detail::pack* pk)
  {
    pack_queue_.push(pk);
  }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(context*, ctx_)
  GCE_CACHE_ALIGNED_VAR(ctxid_t const, ctxid_)
  GCE_CACHE_ALIGNED_VAR(timestamp_t const, timestamp_)
  GCE_CACHE_ALIGNED_VAR(std::size_t const, max_cache_match_size_)
  GCE_CACHE_ALIGNED_VAR(thrid_t, thrid_)
  GCE_CACHE_ALIGNED_VAR(io_service_t, ios_)

  GCE_CACHE_ALIGNED_VAR(detail::cache_pool, cac_pool_)
  GCE_CACHE_ALIGNED_VAR(detail::pack_queue_t, pack_queue_)

  /// thread local vals
  bool stopped_;
  std::size_t const max_wait_counter_size_;
};
}

#endif /// GCE_ACTOR_THREAD_HPP
