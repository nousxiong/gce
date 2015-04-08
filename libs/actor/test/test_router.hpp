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
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
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
      
      threaded_actor base = spawn(ctx);
      threaded_actor base1 = spawn(ctx1);
      threaded_actor base2 = spawn(ctx2);

      netopt_t opt = make_netopt();
      opt.is_router = 1;
      gce::bind(base, "tcp://127.0.0.1:14923", remote_func_list_t(), opt);

      aid_t echo_aid =
        spawn(
          base2,
          boost::bind(
            &socket_ut::echo, _arg1
            ),
          monitored
          );

      opt.reconn_period = seconds(1);
      connect(base1, "router", "tcp://127.0.0.1:14923", opt);
      connect(base2, "router", "tcp://127.0.0.1:14923", opt);
      base2.sleep_for(millisecs(100));

      for (std::size_t i=0; i<echo_num; ++i)
      {
        base1->send(echo_aid, "echo");
        base1->recv("echo");
      }
      base1->send(echo_aid, "end");

      base2->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << "test_base except: " << ex.what() << std::endl;
    }
  }

  static void echo(stackful_actor self)
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
