///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/detail/scope.hpp>

namespace gce
{
class service_ut
{
public:
  static void run()
  {
    std::cout << "service_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "service_ut end." << std::endl;
  }

private:
  static void test_common()
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
      
      net_option opt;
      opt.is_router_ = true;
      gce::bind(base, "tcp://127.0.0.1:14923", remote_func_list_t(), opt);

      spawn(
        base2,
        boost::bind(
          &service_ut::echo_service, _1
          ),
        monitored
        );
      svcid_t echo_svc = make_svcid("two", "echo_svc");

      opt.reconn_period_ = seconds_t(1);
      connect(base1, "router", "tcp://127.0.0.1:14923", opt);
      connect(base2, "router", "tcp://127.0.0.1:14923", opt);
      base2.sleep_for(millisecs_t(100));

      for (std::size_t i=0; i<echo_num; ++i)
      {
        base1->send(echo_svc, "echo");
        base1->recv("echo");
      }
      base1->send(echo_svc, "end");

      base2->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  static void echo_service(stackful_actor self)
  {
    try
    {
      register_service(self, "echo_svc");

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
      deregister_service(self, "echo_svc");
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo except: " << ex.what() << std::endl;
    }
  }
};
}
