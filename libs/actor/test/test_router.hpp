///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

namespace gce
{
class router_ut
{
public:
  static void run()
  {
    std::cout << "router_ut begin." << std::endl;
    test_base();
    std::cout << "router_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      std::size_t echo_num = 100;

      attributes attrs;
      attrs.id_ = atom("router");
      context ctx(attrs);
      attrs.id_ = atom("one");
      context ctx1(attrs);
      attrs.id_ = atom("two");
      context ctx2(attrs);

      mixin_t router = spawn(ctx);
      gce::bind(router, "tcp://127.0.0.1:14923", true);

      mixin_t base1 = spawn(ctx1);
      mixin_t base2 = spawn(ctx2);

      aid_t echo_aid =
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
      connect(base1, atom("router"), "tcp://127.0.0.1:14923", true, opt);
      connect(base2, atom("router"), "tcp://127.0.0.1:14923", true, opt);
      wait(base2, boost::chrono::milliseconds(100));

      for (std::size_t i=0; i<echo_num; ++i)
      {
        send(base1, echo_aid, atom("echo"));
        recv(base1, atom("echo"));
      }
      send(base1, echo_aid, atom("end"));

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
      std::cerr << "echo except: " << ex.what() << std::endl;
    }
  }
};
}
