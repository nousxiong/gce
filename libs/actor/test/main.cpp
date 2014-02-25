///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/all.hpp>
#include <gce/detail/mpsc_queue.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/array.hpp>
#include <boost/ref.hpp>
#include <boost/assert.hpp>
#include <iostream>
#include <string>
#include <vector>

#include <boost/timer/timer.hpp>
#include "test_object_pool.hpp"
#include "test_actor.hpp"
#include "test_slice.hpp"
#include "test_actor_pingpong.hpp"
#include "test_match.hpp"
#include "test_link.hpp"
#include "test_socket.hpp"
#include "test_message.hpp"
#include "test_send_recv.hpp"

int main()
{
  try
  {
    gce::object_pool_ut::run();
    gce::send_recv_ut::run();
    gce::actor_ut::run();
    gce::slice_ut::run();
    gce::actor_pingpong_ut::run();
    gce::match_ut::run();
    gce::link_ut::run();
    gce::socket_ut::run();
    gce::message_ut::run();
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
