///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ATTRIBUTE_HPP
#define GCE_ACTOR_ATTRIBUTE_HPP

#include <gce/actor/config.hpp>
#ifdef GCE_LUA
# include <gce/lualib/all.hpp>
#endif
#include <gce/actor/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>

namespace gce
{
typedef size_t thrid_t;
typedef boost::function<void (thrid_t)> thread_callback_t;
#ifdef GCE_LUA
  typedef boost::function<void (lua_State*)> lua_register_t;
#endif
struct attributes
{
  attributes()
    : ios_(0)
    , id_(ctxid_nil)
    , thread_num_(boost::thread::hardware_concurrency())
    , per_thread_service_num_(1)
    , nonblocked_num_(1)
    , actor_pool_reserve_size_(8)
    , socket_pool_reserve_size_(8)
    , acceptor_pool_reserve_size_(8)
    , max_cache_match_size_(32)
  {
  }

  io_service_t* ios_;
  ctxid_t id_;
  size_t thread_num_;
  size_t per_thread_service_num_;
  size_t nonblocked_num_;
  size_t actor_pool_reserve_size_;
  size_t socket_pool_reserve_size_;
  size_t acceptor_pool_reserve_size_;
  size_t max_cache_match_size_;
  std::vector<thread_callback_t> thread_begin_cb_list_;
  std::vector<thread_callback_t> thread_end_cb_list_;
#ifdef GCE_LUA
  std::vector<std::string> lua_gce_path_list_;
  std::vector<lua_register_t> lua_reg_list_;
#endif
  log::logger_t lg_;
};
}

#endif
