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
#include <gce/actor/response.hpp>
#include <gce/actor/detail/actor_pool.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <boost/optional.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>
#include <set>
#include <map>

namespace gce
{
class context;
class coroutine_stackful_actor;
class coroutine_stackless_actor;
#ifdef GCE_LUA
class lua_actor;
#endif
struct attributes;

namespace detail
{
class socket;
class acceptor;
struct pack;

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

#ifdef GCE_LUA
  inline lua_State* get_lua_state() { return L_.get(); }
  void init_lua(std::string const& gce_path);
  luabridge::LuaRef get_script(std::string const& file);
#endif

  coroutine_stackful_actor* make_stackful_actor();
  coroutine_stackless_actor* make_stackless_actor();
#ifdef GCE_LUA
  lua_actor* make_lua_actor();
  aid_t spawn_lua_actor(std::string const& script, aid_t sire, link_type);
#endif
  socket* make_socket();
  acceptor* make_acceptor();

  void free_actor(coroutine_stackful_actor*);
  void free_actor(coroutine_stackless_actor*);
#ifdef GCE_LUA
  void free_actor(lua_actor*);
#endif
  void free_socket(socket*);
  void free_acceptor(acceptor*);

  void on_recv(
    detail::actor_index, sid_t sid, 
    detail::pack&, detail::send_hint
    );

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

  void send_already_exited(aid_t recver, aid_t sender);
  void send_already_exited(aid_t recver, resp_t res);
  void send(aid_t const& recver, detail::pack&, detail::send_hint);
  aid_t filter_aid(aid_t const& src);
  aid_t filter_svcid(svcid_t const& src);

private:
  void handle_recv(
    detail::actor_index i, sid_t sid, 
    detail::pack&, detail::send_hint
    );
  void send_already_exit(detail::pack&);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(context*, ctx_)
  GCE_CACHE_ALIGNED_VAR(std::size_t, index_)
  GCE_CACHE_ALIGNED_VAR(strand_t, snd_)

#ifdef GCE_LUA
  GCE_CACHE_ALIGNED_VAR(detail::unique_ptr<lua_State>, L_)
#endif

  /// pools
  struct pool_impl;
  GCE_CACHE_ALIGNED_VAR(boost::scoped_ptr<pool_impl>, pool_list_)

  /// thread local vals
  ctxid_t const ctxid_;
  timestamp_t const timestamp_;
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

#ifdef GCE_LUA
  typedef std::map<std::string, luabridge::LuaRef> script_list_t;
  script_list_t script_list_;
#endif

  bool stopped_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_CACHE_POOL_HPP
