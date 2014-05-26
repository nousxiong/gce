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
    test_common();
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

      gce::bind(ctx, "tcp://127.0.0.1:14923", true);

      spawn(
        ctx2,
        boost::bind(
          &service_ut::echo_service, _1
          ),
        monitored
        );
      svcid_t echo_svc(atom("two"), atom("echo_svc"));

      wait(ctx1, boost::chrono::milliseconds(100));
      net_option opt;
      opt.reconn_period_ = seconds_t(1);
      connect(ctx1, atom("router"), "tcp://127.0.0.1:14923", true, opt);
      connect(ctx2, atom("router"), "tcp://127.0.0.1:14923", true, opt);
      wait(ctx2, boost::chrono::milliseconds(100));

      for (std::size_t i=0; i<echo_num; ++i)
      {
        send(ctx1, echo_svc, atom("echo"));
        recv(ctx1, atom("echo"));
      }
      send(ctx1, echo_svc, atom("end"));

      recv(ctx2);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  static void echo_service(actor<stackful>& self)
  {
    try
    {
      register_service(self, atom("echo_svc"));

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
      deregister_service(self, atom("echo_svc"));
    }
    catch (std::exception& ex)
    {
      std::cerr << "echo except: " << ex.what() << std::endl;
    }
  }
};
}
