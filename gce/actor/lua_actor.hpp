///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_LUA_ACTOR_HPP
#define GCE_ACTOR_LUA_ACTOR_HPP

#include <gce/actor/config.hpp>

#ifdef GCE_LUA

#include <gce/actor/basic_actor.hpp>
#include <gce/actor/pattern.hpp>
#include <gce/actor/detail/mailbox_fwd.hpp>

namespace gce
{
namespace detail
{
class cache_pool;
class mailbox;
}

class lua_actor
  : public basic_actor
{
  typedef basic_actor base_type;

public:
  inline void send(aid_t recver, message const& m)
  {
    base_type::pri_send(recver, m);
  }

  inline void send2svc(svcid_t recver, message const& m)
  {
    base_type::pri_send_svc(recver, m);
  }

  inline void relay(aid_t des, message& m)
  {
    base_type::pri_relay(des, m);
  }

  inline void relay2svc(svcid_t des, message& m)
  {
    base_type::pri_relay_svc(des, m);
  }

  inline resp_t request(aid_t recver, message const& m)
  {
    resp_t res(new_request(), get_aid(), recver);
    base_type::pri_request(res, recver, m);
    return res;
  }

  inline resp_t request2svc(svcid_t recver, message const& m)
  {
    resp_t res(new_request(), get_aid(), recver);
    base_type::pri_request_svc(res, recver, m);
    return res;
  }

  inline void reply(aid_t recver, message const& m)
  {
    base_type::pri_reply(recver, m);
  }

  inline void link(aid_t target)
  {
    base_type::pri_link(target);
  }

  inline void monitor(aid_t target)
  {
    base_type::pri_monitor(target);
  }

public:
  lua_actor(aid_t aid, detail::cache_pool*);
  ~lua_actor();

public:
  void recv();
  void recv_match(pattern const& patt);
  void recv_response(resp_t);
  void recv_response_timeout(resp_t, duration_type tmo);
  void wait(duration_type);
  void spawn(std::string const& script, bool sync_sire, int);
  void spawn_remote(
    detail::spawn_type, std::string const& func, match_type ctxid, 
    int type, std::size_t stack_size, seconds_t tmo
    );

public:
  /// internal use
  static detail::actor_type type() { return detail::actor_lua; }
  void set_coro(luabridge::LuaRef co);
  void init(std::string const& script);
  void start();
  void on_recv(detail::pack&, detail::send_hint);
  void quit(exit_code_t exc = exit_normal, std::string const& errmsg = std::string());

private:
  void yield();
  void resume();
  void stop(aid_t self_aid, exit_code_t, std::string const&);

  void start_timer(duration_t);
  void handle_timeout(errcode_t const&, std::size_t);

  aid_t end_recv(detail::recv_t&, message&);
  aid_t end_recv(resp_t&);

  void handle_recv(detail::pack&);
  void handle_spawn(aid_t, link_type);
  void handle_remote_spawn(
    aid_t aid,
    message msg, link_type type,
    boost::chrono::system_clock::time_point begin_tp,
    sid_t sid, seconds_t tmo, duration_t curr_tmo
    );

  void set_recv_global(aid_t const& sender, message const& msg);

private:
  /// Ensure start from a new cache line.
  byte_t pad0_[GCE_CACHE_LINE_SIZE];

  GCE_CACHE_ALIGNED_VAR(lua_State*, L_)
  GCE_CACHE_ALIGNED_VAR(std::string, script_)
  GCE_CACHE_ALIGNED_VAR(luabridge::LuaRef, f_)
  GCE_CACHE_ALIGNED_VAR(luabridge::LuaRef, co_)

  /// thread local
  bool yielding_;
  bool recving_;
  bool responsing_;
  resp_t recving_res_;
  pattern curr_pattern_;
  timer_t tmr_;
  std::size_t tmr_sid_;

  typedef boost::function<void (aid_t, message)> spawn_handler_t;
  spawn_handler_t spw_hdr_;
};
}

#endif /// GCE_LUA
#endif /// GCE_ACTOR_LUA_ACTOR_HPP
