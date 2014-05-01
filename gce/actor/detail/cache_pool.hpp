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
#include <gce/actor/detail/pack.hpp>
#include <gce/detail/mpsc_queue.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <set>
#include <map>

namespace gce
{
class thread;
class context;
class actor;
struct attributes;

namespace detail
{
class socket;
class acceptor;
typedef object_pool<actor, thread*> actor_pool_t;
typedef object_pool<socket, thread*> socket_pool_t;
typedef object_pool<acceptor, thread*> acceptor_pool_t;
typedef mpsc_queue<actor> actor_queue_t;
typedef mpsc_queue<socket> socket_queue_t;
typedef mpsc_queue<acceptor> acceptor_queue_t;

class cache_pool
  : private boost::noncopyable
{
public:
  cache_pool(thread&, attributes const& attrs);
  ~cache_pool();

public:
  actor* get_actor();
  socket* get_socket();
  acceptor* get_acceptor();
  pack* get_pack();

  void free_actor(actor*);
  void free_socket(socket*);
  void free_acceptor(acceptor*);
  void free_pack(pack*);

  void free_object();

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
  template <typename T, typename Pool, typename FreeQueue>
  void free_object(Pool&, FreeQueue&);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(thread*, thr_)

  /// pools
  GCE_CACHE_ALIGNED_VAR(actor_pool_t, actor_pool_)
  GCE_CACHE_ALIGNED_VAR(socket_pool_t, socket_pool_)
  GCE_CACHE_ALIGNED_VAR(acceptor_pool_t, acceptor_pool_)
  GCE_CACHE_ALIGNED_VAR(pack_pool_t, pack_pool_)

  GCE_CACHE_ALIGNED_VAR(pack_queue_t, pack_free_queue_)

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
