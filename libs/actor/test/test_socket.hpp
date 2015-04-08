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
class socket_ut
{
public:
  static void run()
  {
    std::cout << "socket_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "socket_ut end." << std::endl;
  }

public:
  static void test_base()
  {
    try
    {
      gce::log::asio_logger lg;
      std::size_t echo_num = 10;

      attributes attrs;
      attrs.lg_ = boost::bind(&gce::log::asio_logger::output, &lg, _arg1, "");
      attrs.id_ = atom("one");
      context ctx1(attrs);
      attrs.id_ = atom("two");
      context ctx2(attrs);
      
      threaded_actor base1 = spawn(ctx1);
      threaded_actor base2 = spawn(ctx2);

      gce::bind(base2, "tcp://127.0.0.1:14923");

      aid_t echo_aid =
        spawn(
          base2,
          boost::bind(
            &socket_ut::echo, _arg1
            ),
          monitored
          );

      netopt_t opt = make_netopt();
      opt.reconn_period = seconds(1);
      connect(base1, "two", "tcp://127.0.0.1:14923", opt);

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
