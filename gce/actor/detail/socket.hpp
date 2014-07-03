///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_SOCKET_HPP
#define GCE_ACTOR_DETAIL_SOCKET_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/heartbeat.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/basic_socket.hpp>
#include <gce/actor/net_option.hpp>
#include <gce/actor/detail/object_pool.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/detail/buffer_ref.hpp>
#include <map>
#include <deque>

namespace gce
{
class thread_mapped_actor;
namespace detail
{
class cache_pool;

class socket
  : public object_pool<socket, cache_pool*>::object
  , public basic_actor
{
  typedef basic_actor base_type;

  enum status
  {
    ready = 0,
    on,
    off,
  };

public:
  explicit socket(cache_pool*);
  ~socket();

public:
  void init(net_option);
  void connect(
    aid_t sire, remote_func_list_t const&, ctxid_t target,
    std::string const&, bool target_is_router
    );

  inline void send(aid_t recver, message const& msg)
  {
    base_type::pri_send(recver, msg);
  }

public:
  void start(std::map<match_t, remote_func> const&, socket_ptr, bool is_router);
  void stop();
  void on_free();
  void on_recv(pack&, base_type::send_hint);

  void link(aid_t) {}
  void monitor(aid_t) {}

private:
  void handle_net_msg(message&);
  void spawn_remote_actor(cache_pool*, spawn_t, remote_func);
  void end_spawn_remote_actor(spawn_t, aid_t aid);
  void send_spawn_ret(spawn_t*, pack&, spawn_error, aid_t aid, bool is_err_ret);
  void send(message const&);
  void send_msg(message const&);
  void send_msg_hb();

  void run_conn(aid_t sire, ctxid_pair_t, std::string const&, yield_t);
  void run(socket_ptr, yield_t);

  socket_ptr make_socket(std::string const&);
  void handle_recv(pack&);
  void add_straight_link(aid_t src, aid_t des);
  void remove_straight_link(aid_t src, aid_t des);
  void add_router_link(aid_t src, aid_t des, sktaid_t skt);
  sktaid_t remove_router_link(aid_t src, aid_t des);

  void on_neterr(aid_t self_aid, errcode_t ec = errcode_t());
  ctxid_pair_t sync_ctxid(ctxid_pair_t new_pr, ctxid_pair_t curr_pr);

private:
  bool parse_message(message&);
  void connect(yield_t, bool init = false);
  errcode_t recv(message&, yield_t);
  void close();
  void reconn();
  template <typename F>
  void start_heartbeat(F);
  void free_self(
    ctxid_pair_t, exit_code_t,
    std::string const&, yield_t
    );

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(status, stat_)
  GCE_CACHE_ALIGNED_VAR(net_option, opt_)

  /// thread local vals
  socket_ptr skt_;
  heartbeat hb_;
  timer_t sync_;
  std::size_t tmr_sid_;

  byte_t recv_buffer_[GCE_SOCKET_RECV_CACHE_SIZE];
  detail::buffer_ref recv_cache_;

  bool conn_;
  std::deque<message> conn_cache_;
  std::size_t curr_reconn_;

  /// remote links
  typedef std::map<aid_t, std::set<aid_t> > straight_link_list_t;
  straight_link_list_t straight_link_list_;
  std::set<aid_t> straight_dummy_;

  typedef std::map<aid_t, std::map<aid_t, sktaid_t> > router_link_list_t;
  router_link_list_t router_link_list_;
  std::map<aid_t, sktaid_t> router_dummy_;

  /// remote spawn's funcs
  typedef std::map<match_t, remote_func> remote_list_t;
  remote_list_t remote_list_;

  bool is_router_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_SOCKET_HPP
