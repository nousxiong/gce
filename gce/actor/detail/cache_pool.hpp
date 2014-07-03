///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_CACHE_POOL_HPP
#define GCE_ACTOR_DETAIL_CACHE_POOL_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor_fwd.hpp>
#include <gce/actor/actor_id.hpp>
#include <gce/actor/detail/object_pool.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <set>
#include <map>

namespace gce
{
class context;
class coroutine_stackful_actor;
class coroutine_stackless_actor;
struct attributes;

namespace detail
{
class socket;
class acceptor;
class cache_pool;
typedef object_pool<coroutine_stackful_actor, cache_pool*> context_switching_actor_pool_t;
typedef object_pool<coroutine_stackless_actor, cache_pool*> event_based_actor_pool_t;
typedef object_pool<socket, cache_pool*> socket_pool_t;
typedef object_pool<acceptor, cache_pool*> acceptor_pool_t;

class cache_pool
  : private boost::noncopyable
{
public:
  cache_pool(context& ctx, std::size_t index, bool is_slice = false);
  ~cache_pool();

public:
  inline context& get_context() { return *ctx_; }
  inline std::size_t get_index() { return index_; }
  inline strand_t& get_strand() { return snd_; }

  coroutine_stackful_actor* get_context_switching_actor();
  coroutine_stackless_actor* get_event_based_actor();
  socket* get_socket();
  acceptor* get_acceptor();

  void free_actor(coroutine_stackful_actor*);
  void free_actor(coroutine_stackless_actor*);
  void free_socket(socket*);
  void free_acceptor(acceptor*);

  void register_service(match_t name, aid_t svc);
  aid_t find_service(match_t name);
  void deregister_service(match_t name, aid_t svc);

  void register_socket(ctxid_pair_t, aid_t skt);
  aid_t select_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0);
  aid_t select_straight_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0);
  aid_t select_joint_socket(ctxid_t ctxid = ctxid_nil, ctxid_t* target = 0);
  aid_t select_router(ctxid_t* target = 0);
  void deregister_socket(ctxid_pair_t, aid_t skt);

  void add_socket(socket*);
  void add_acceptor(acceptor*);
  void remove_socket(socket*);
  void remove_acceptor(acceptor*);

  void stop();
  inline bool stopped() const { return stopped_; }

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(context*, ctx_)
  GCE_CACHE_ALIGNED_VAR(std::size_t, index_)
  GCE_CACHE_ALIGNED_VAR(strand_t, snd_)

  /// pools
  GCE_CACHE_ALIGNED_VAR(boost::optional<context_switching_actor_pool_t>, context_switching_actor_pool_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<event_based_actor_pool_t>, event_based_actor_pool_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<socket_pool_t>, socket_pool_)
  GCE_CACHE_ALIGNED_VAR(boost::optional<acceptor_pool_t>, acceptor_pool_)

  /// thread local vals
  typedef std::set<aid_t> skt_list_t;
  struct socket_list
  {
    skt_list_t skt_list_;
    skt_list_t::iterator curr_skt_;
  };

  typedef std::map<ctxid_t, socket_list> conn_list_t;
  conn_list_t conn_list_;
  conn_list_t joint_list_;
  conn_list_t router_list_;
  conn_list_t::iterator curr_router_list_;
  conn_list_t::iterator curr_socket_list_;
  conn_list_t::iterator curr_joint_list_;
  socket_list dummy_;

  typedef std::map<match_t, aid_t> service_list_t;
  service_list_t service_list_;

  std::set<socket*> socket_list_;
  std::set<acceptor*> acceptor_list_;

  bool stopped_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_CACHE_POOL_HPP
