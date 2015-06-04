///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ACTOR_FWD_HPP
#define GCE_ACTOR_ACTOR_FWD_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/atom.hpp>
#include <gce/actor/context_id.hpp>
#include <gce/actor/match.hpp>
#include <boost/coroutine/stack_traits.hpp>
#include <utility>

namespace gce
{
/// actor using stack's default size
inline size_t default_stacksize()
{
  return boost::coroutines::stack_traits::default_size();
}

/// actor using stack's minimum size
inline size_t minimum_stacksize()
{
  return boost::coroutines::stack_traits::minimum_size();
}

typedef uint32_t sid_t;
static uint32_t const sid_nil = static_cast<sid_t>(-1);

/// actor type tags
struct threaded {};
struct stackful {};
struct stackless {};
struct nonblocked {};
struct socket {};
struct acceptor {};
struct addon {};
#ifdef GCE_LUA
struct luaed {};
#endif

/// actor link type
enum link_type
{
  no_link = 0,
  linked, /// monitor each other
  monitored /// only monitor target
};

/// actor exit error code
typedef match_t exit_code_t;
static exit_code_t const exit = atom("gce_exit");
static exit_code_t const exit_normal = atom("gce_ex_normal");
static exit_code_t const exit_except = atom("gce_ex_except");
static exit_code_t const exit_remote = atom("gce_ex_remote");
static exit_code_t const exit_already = atom("gce_ex_alread");
static exit_code_t const exit_neterr = atom("gce_ex_neterr");

namespace detail
{
enum actor_type
{
  actor_nil = -1, 

  actor_threaded,
  actor_stackful,
  actor_stackless,
  actor_nonblocked,
  actor_socket,
  actor_acceptor,
  actor_addon,
#ifdef GCE_LUA
  actor_luaed,
#endif

  actor_num
};

static match_t const msg_hb = atom("gce_msg_hb");
static match_t const msg_reg_skt = atom("gce_reg_skt");
static match_t const msg_dereg_skt = atom("gce_dereg_skt");
static match_t const msg_reg_svc = atom("gce_reg_svc");
static match_t const msg_dereg_svc = atom("gce_dereg_svc");
static match_t const msg_add_svc = atom("gce_add_svc");
static match_t const msg_rmv_svc = atom("gce_rmv_svc");
static match_t const msg_login = atom("gce_login");
static match_t const msg_login_ret = atom("gce_login_ret");
static match_t const msg_link = atom("gce_link");
static match_t const msg_send = atom("gce_send");
static match_t const msg_relay = atom("gce_relay");
static match_t const msg_request = atom("gce_request");
static match_t const msg_reply = atom("gce_reply");
static match_t const msg_stop = atom("gce_stop");
static match_t const msg_spawn = atom("gce_spawn");
static match_t const msg_spawn_ret = atom("gce_spawn_ret");
static match_t const msg_new_actor = atom("gce_new_actor");
static match_t const msg_new_conn = atom("gce_new_conn");
static match_t const msg_new_bind = atom("gce_new_bind");

enum socket_type
{
  socket_comm = 0,
  socket_router,
  socket_joint
};

typedef std::pair<gce::ctxid_t, socket_type> ctxid_pair_t;

enum spawn_type
{
  spw_nil = 0,
  spw_stackful,
  spw_stackless,
#ifdef GCE_LUA
  spw_luaed,
#endif
};
} /// namespace detail
} /// namespace gce

#ifndef GCE_DEFAULT_REQUEST_TIMEOUT_SEC
# define GCE_DEFAULT_REQUEST_TIMEOUT_SEC 180
#endif

#endif /// GCE_ACTOR_ACTOR_FWD_HPP
