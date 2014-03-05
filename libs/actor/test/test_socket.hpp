///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/actor.hpp>
#include <gce/actor/detail/socket.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/spawn.hpp>
#include <gce/actor/remote.hpp>
#include <gce/actor/atom.hpp>
#include <gce/amsg/amsg.hpp>
#include <boost/atomic.hpp>

namespace gce
{
class socket_ut
{
public:
  static void run()
  {
    std::cout << "socket_ut begin." << std::endl;
    test_base();
    std::cout << "socket_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      attributes attrs;
      attrs.id_ = atom("ctx_one");
      context ctx1(attrs);
      attrs.id_ = atom("ctx_two");
      context ctx2(attrs);

      mixin_t base1 = spawn(ctx1);
      mixin_t base2 = spawn(ctx2);

//      base1.set_ctxid(atom("ctx_one"));
//      base2.set_ctxid(atom("ctx_two"));

      recv(base1, zero);
      recv(base2, zero);

      remote_func_list_t func_list;
      func_list.push_back(
        std::make_pair(
          atom("echo_client"),
          boost::bind(&socket_ut::dummy, _1)
          )
        );
      gce::bind(base2, "tcp://127.0.0.1:14923", func_list);

      //wait(base1, boost::chrono::milliseconds(200));
      net_option opt;
      opt.reconn_period_ = seconds_t(5);
      connect(base1, atom("ctx_two"), "tcp://127.0.0.1:14923", opt);

      spawn(
        base1,
        boost::bind(
          &socket_ut::dummy, _1
          ),
        monitored
        );

      recv(base1);
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void dummy(self_t self)
  {
    try
    {
      connect(self, atom("null"), "tcp://127.0.0.1:14923");
      connect(self, atom("ctx_two"), "tcp://127.0.0.1:14923");
    }
    catch (std::exception& ex)
    {
      std::cerr << "dummy except: " << ex.what() << std::endl;
    }
  }
};
}
