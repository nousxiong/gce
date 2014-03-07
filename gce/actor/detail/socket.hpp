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
#include <gce/detail/mpsc_queue.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <gce/actor/detail/buffer_ref.hpp>
#include <map>
#include <deque>

namespace gce
{
class mixin;
namespace detail
{
class cache_pool;

class socket
  : public object_pool<socket, context*>::object
  , public mpsc_queue<socket>::node
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
  explicit socket(context*);
  ~socket();

public:
  void init(cache_pool* user, cache_pool* owner, net_option);
  void connect(ctxid_t target, std::string const&);

public:
  void start(std::map<match_t, actor_func_t> const&, socket_ptr);
  void stop();
  void on_free();
  void on_recv(pack*);

  void link(aid_t) {}
  void monitor(aid_t) {}

private:
  ctxid_t handle_net_msg(message&, ctxid_t curr_ctxid);
  void send(detail::pack*);
  void send(message const&);
  void send_msg(message const&);
  void send_msg_hb();

  void run_conn(ctxid_t target, std::string const&, yield_t);
  void run(socket_ptr, yield_t);

  socket_ptr make_socket(std::string const&);
  void handle_recv(pack*);
  void add_straight_link(aid_t src, aid_t des);
  void remove_straight_link(aid_t src, aid_t des);

  void on_neterr(errcode_t ec = errcode_t());
  ctxid_t sync_ctxid(ctxid_t new_ctxid, ctxid_t curr_ctxid);

private:
  bool parse_message(message&);
  void connect(yield_t);
  errcode_t recv(message&, yield_t);
  void close();
  void reconn();
  template <typename F>
  void start_heartbeat(F);
  void free_self(
    ctxid_t, exit_code_t,
    std::string const&, yield_t
    );

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

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
  std::set<aid_t> dummy_;

  /// remote spawn's funcs
  std::map<match_t, actor_func_t> remote_list_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_SOCKET_HPP
