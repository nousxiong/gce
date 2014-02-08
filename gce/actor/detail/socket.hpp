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
#include <gce/actor/message.hpp>
#include <gce/actor/detail/heartbeat.hpp>
#include <gce/actor/basic_actor.hpp>
#include <gce/actor/detail/object_pool.hpp>
#include <gce/detail/mpsc_queue.hpp>
#include <gce/actor/detail/pack.hpp>
#include <gce/actor/match.hpp>
#include <gce/detail/asio_alloc_handler.hpp>
#include <gce/detail/cache_aligned_new.hpp>
#include <gce/actor/detail/buffer_ref.hpp>
#include <queue>
#include <deque>

namespace gce
{
class mixin;
namespace detail
{
class cache_pool;
class basic_socket;

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
  void connect(std::string const&, aid_t);

public:
  void start(basic_socket*, aid_t, aid_t);
  void on_free();
  void on_recv(pack*);

  void link(aid_t) {}
  void monitor(aid_t) {}

private:
  void send(aid_t, message const&);
  void send(message const&);
  void send_msg(message const&);
  void send_msg_hb();
  void run_conn(std::string const&, yield_t);
  void run(basic_socket*, yield_t);
  basic_socket* make_socket(std::string const&);
  void handle_recv(pack*);

private:
  bool parse_message(message&);
  void connect(yield_t);
  errcode_t recv(message&, yield_t);
  void close();
  void reconn();
  template <typename F>
  void start_heartbeat(F);
  void free_self(exit_code_t, yield_t);

private:
  byte_t pad0_[GCE_CACHE_LINE_SIZE]; /// Ensure start from a new cache line.

  status stat_;
  byte_t pad1_[GCE_CACHE_LINE_SIZE - sizeof(status)];

  cache_aligned_ptr<cache_pool, cache_pool*> user_;

  net_option opt_;
  byte_t pad2_[GCE_CACHE_LINE_SIZE - sizeof(net_option)];

  /// thread local vals
  basic_socket* skt_;
  heartbeat hb_;
  timer_t sync_;
  std::size_t tmr_sid_;

  aid_t master_;
  aid_t acpr_aid_;

  byte_t recv_buffer_[GCE_SOCKET_RECV_CACHE_SIZE];
  detail::buffer_ref recv_cache_;

  bool conn_;
  std::queue<message, std::deque<message> > conn_cache_;
  std::size_t curr_reconn_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_SOCKET_HPP
