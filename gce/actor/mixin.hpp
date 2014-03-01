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
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/optional.hpp>
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
  typedef basic_actor base_type;

public:
  mixin(context& ctx, std::size_t id, attributes const& attrs);
  ~mixin();

public:
  aid_t recv(message&, match const& mach = match());
  void send(aid_t, message const&);
  void relay(aid_t, message&);

  response_t request(aid_t, message const&);
  void reply(aid_t, message const&);
  aid_t recv(response_t, message&, duration_t);
  void wait(duration_t);

  void link(aid_t);
  void monitor(aid_t);

public:
  detail::cache_pool* select_cache_pool();
  std::size_t get_cache_match_size() const;
  void on_recv(detail::pack*);
  void gc();

  slice* get_slice();
  void free_slice(slice*);
  void free_cache();

private:
  void delete_cache();

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  /// select cache pool
  GCE_CACHE_ALIGNED_VAR(std::size_t, curr_cache_pool_)
  GCE_CACHE_ALIGNED_VAR(std::size_t, cache_pool_size_)

  GCE_CACHE_ALIGNED_VAR(boost::shared_mutex, mtx_)
  GCE_CACHE_ALIGNED_VAR(boost::condition_variable_any, cv_)

  GCE_CACHE_ALIGNED_VAR(std::vector<detail::cache_pool*>, cache_pool_list_)

  /// pools
  GCE_CACHE_ALIGNED_VAR(boost::optional<detail::slice_pool_t>, slice_pool_)
};

typedef mixin& mixin_t;
}

#endif /// GCE_ACTOR_MIXIN_HPP
