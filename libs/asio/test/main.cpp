///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/all.hpp>
#include <gce/asio/all.hpp>
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
#include "test_timer.hpp"
#include "test_signal.hpp"
#include "test_tcp.hpp"
#ifdef GCE_OPENSSL
# include "test_ssl.hpp"
#endif
#ifdef GCE_LUA
# include "test_lua_asio.hpp"
#endif

int main()
{
  try
  {
    /// basic test
    gce::asio::timer_ut::run();
    gce::asio::signal_ut::run();
    gce::asio::tcp_ut::run();

#ifdef GCE_OPENSSL
    gce::asio::ssl_ut::run();
#endif

    /// script test
#ifdef GCE_LUA
    gce::lua_asio_ut::run();
#endif
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
