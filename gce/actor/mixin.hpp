///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_MIXIN_HPP
#define GCE_ACTOR_MIXIN_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/context.hpp>
#include <gce/detail/asio_alloc_handler.hpp>
#include <gce/detail/cache_aligned_new.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <vector>

namespace gce
{
class slice;
class mixin;
namespace detail
{
class cache_pool;
typedef object_pool<slice, mixin*> slice_pool_t;
}

class mixin
  : public basic_actor
{
public:
  mixin(context& ctx, std::size_t id, attributes const& attrs);
  ~mixin();

public:
  aid_t recv(message&, match const& mach = match());
  void send(aid_t, message const&);
  void reply(aid_t, message const&);

  detail::cache_pool* select_cache_pool();
  std::size_t get_cache_match_size() const;
  void link(aid_t);
  void monitor(aid_t);

  void on_recv(detail::pack*);
  void gc();
  inline detail::slice_pool_t* get_slice_pool() { return slice_pool_.get(); }
  void free_slice(slice*);
  void free_cache();

private:
  void delete_cache();

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  std::size_t curr_cache_pool_; /// select cache pool
  byte_t pad1_[GCE_CACHE_LINE_SIZE - sizeof(std::size_t)];

  std::size_t cache_pool_size_;
  byte_t pad2_[GCE_CACHE_LINE_SIZE - sizeof(std::size_t)];

  typedef detail::unique_ptr<boost::shared_mutex> mutex_ptr;
  detail::cache_aligned_ptr<boost::shared_mutex, mutex_ptr> mtx_;

  typedef detail::unique_ptr<boost::condition_variable_any> cv_ptr;
  detail::cache_aligned_ptr<boost::condition_variable_any, cv_ptr> cv_;

  typedef detail::cache_aligned_ptr<detail::cache_pool, detail::cache_pool*> cache_pool_ptr;
  typedef std::vector<cache_pool_ptr> cache_pool_list_t;
  typedef detail::unique_ptr<cache_pool_list_t> cache_pool_list_ptr;
  detail::cache_aligned_ptr<cache_pool_list_t, cache_pool_list_ptr> cache_pool_list_;

  /// pools
  typedef detail::unique_ptr<detail::slice_pool_t> slice_pool_ptr;
  detail::cache_aligned_ptr<detail::slice_pool_t, slice_pool_ptr> slice_pool_;
};

typedef mixin& mixin_t;
}

#endif /// GCE_ACTOR_MIXIN_HPP
