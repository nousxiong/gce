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
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

namespace gce
{
class slice;
class mixin;
class context;
struct attributes;
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
  inline context& get_context() { return *ctx_; }

  aid_t recv(message&, match const& mach = match());
  aid_t recv(
    response_t, message&,
    duration_t tmo = seconds_t(GCE_DEFAULT_REQUEST_TIMEOUT_SEC)
    );
  void wait(duration_t);
  void update();

public:
  /// internal use
  detail::cache_pool* get_cache_pool();
  static void move_pack(
    basic_actor* base,
    detail::mailbox&,
    detail::pack_queue_t&,
    detail::cache_pool*,
    mixin*
    );
  void on_recv(detail::pack*);
  void gc();

  slice* get_slice();
  void free_slice(slice*);
  void free_cache();

  void register_service(match_t name, aid_t svc, detail::cache_pool*);
  void deregister_service(match_t name, aid_t svc, detail::cache_pool*);

  void register_socket(
    ctxid_pair_t ctxid_pr,
    aid_t skt, detail::cache_pool*
    );
  void deregister_socket(
    ctxid_pair_t ctxid_pr,
    aid_t skt, detail::cache_pool*
    );
  void stop(detail::cache_pool*);

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  GCE_CACHE_ALIGNED_VAR(context*, ctx_)

  GCE_CACHE_ALIGNED_VAR(boost::shared_mutex, mtx_)
  GCE_CACHE_ALIGNED_VAR(boost::condition_variable_any, cv_)

  GCE_CACHE_ALIGNED_VAR(boost::scoped_ptr<detail::cache_pool>, cac_pool_)

  /// pools
  GCE_CACHE_ALIGNED_VAR(boost::optional<detail::slice_pool_t>, slice_pool_)

  /// local
  ctxid_t ctxid_;
};

typedef mixin& mixin_t;
}

#endif /// GCE_ACTOR_MIXIN_HPP
