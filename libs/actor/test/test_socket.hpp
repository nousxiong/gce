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
  static void dummy(self_t)
  {
  }

  static void test_base()
  {
    try
    {
      std::size_t echo_num = 100;

      attributes attrs;
      attrs.id_ = atom("ctx_one");
      context ctx1(attrs);
      attrs.id_ = atom("ctx_two");
      context ctx2(attrs);

      mixin_t base1 = spawn(ctx1);
      mixin_t base2 = spawn(ctx2);

      recv(base1, zero);
      recv(base2, zero);

      remote_func_list_t func_list;
      func_list.push_back(
        std::make_pair(
          atom("echo_client"),
          boost::bind(
            &socket_ut::dummy, _1
            )
          )
        );
      gce::bind(base2, "tcp://127.0.0.1:14923", func_list);

      aid_t dummy_id =
        spawn(
          base2,
          boost::bind(
            &socket_ut::echo, _1
            ),
          monitored
          );

      wait(base1, boost::chrono::milliseconds(100));
      net_option opt;
      opt.reconn_period_ = seconds_t(1);
      connect(base1, atom("ctx_two"), "tcp://127.0.0.1:14923", opt);

      for (std::size_t i=0; i<echo_num; ++i)
      {
        send(base1, dummy_id, atom("echo"));
        recv(base1, atom("echo"));
      }
      send(base1, dummy_id, atom("end"));

      recv(base2);
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void echo(self_t self)
  {
    try
    {
      while (true)
      {
        message msg;
        aid_t sender = self.recv(msg);
        match_t type = msg.get_type();
        if (type == atom("echo"))
        {
          self.send(sender, msg);
        }
        else
        {
          break;
        }
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << "dummy except: " << ex.what() << std::endl;
    }
  }
};
}
