///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "arg.adl.h"
#include "echo.adl.h"
#include <gce/actor/all.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <gce/detail/dynarray.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/array.hpp>
#include <boost/ref.hpp>
#include <boost/assert.hpp>
#include <iostream>
#include <string>
#include <vector>

static std::size_t const test_count = 1;

#include <boost/timer/timer.hpp>
#include "test_coro.hpp"
#include "test_actor.hpp"
#include "test_response.hpp"
#include "test_stackless.hpp"
#include "test_mixin.hpp"
#include "test_slice.hpp"
#include "test_actor_pingpong.hpp"
#include "test_stackless_pingpong.hpp"
#include "test_mixin_pingpong.hpp"
#include "test_slice_pingpong.hpp"
#include "test_match.hpp"
#include "test_match_recver.hpp"
#include "test_link.hpp"
#include "test_message.hpp"
#include "test_pointer.hpp"
#include "test_relay.hpp"
#include "test_common_relay.hpp"
#include "test_addon.hpp"
#include "test_socket.hpp"
#include "test_socket_broken.hpp"
#include "test_big_msg.hpp"
#include "test_remote_link.hpp"
#include "test_router.hpp"
#include "test_router_link.hpp"
#include "test_router_broken.hpp"
#include "test_remote.hpp"
#include "test_remote_relay.hpp"
#include "test_remote_common_relay.hpp"
#include "test_send_recv.hpp"
#include "test_service.hpp"
#include "test_services.hpp"
#ifdef GCE_LUA
# include "test_lua_actor.hpp"
# include "test_lua_socket.hpp"
# include "test_lua_service.hpp"
# include "test_lua_services.hpp"
#endif

int main()
{
  try
  {
    /// basic test
    gce::coro_ut::run();
    gce::send_recv_ut::run();
    gce::actor_ut::run();
    gce::response_ut::run();
    gce::stackless_ut::run();
    gce::mixin_ut::run();
    gce::slice_ut::run();
    gce::actor_pingpong_ut::run();
    gce::stackless_pingpong_ut::run();
    gce::mixin_pingpong_ut::run();
    gce::slice_pingpong_ut::run();
    gce::match_ut::run();
    gce::match_recver_ut::run();
    gce::link_ut::run();
    gce::relay_ut::run();
    gce::common_relay_ut::run();
    gce::message_ut::run();
    gce::pointer_ut::run();
    gce::addon_ut::run();

    /// remote test
    gce::socket_ut::run();
    gce::socket_broken_ut::run();
    gce::big_msg_ut::run();
    gce::remote_ut::run();
    gce::remote_link_ut::run();
    gce::router_ut::run();
    gce::router_link_ut::run();
    gce::router_broken_ut::run();
    gce::remote_relay_ut::run();
    gce::remote_common_relay_ut::run();
    gce::service_ut::run();
    gce::services_ut::run();

    /// script test
#ifdef GCE_LUA
    gce::lua_actor_ut::run();
    gce::lua_socket_ut::run();
    gce::lua_service_ut::run();
    gce::lua_services_ut::run();
#endif
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
