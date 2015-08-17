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
class service_link_ut
{
public:
  static void run()
  {
    std::cout << "service_link_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "service_link_ut end." << std::endl;
  }

private:
  static void test_common()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");
    try
    {
      size_t const echo_num = 10;
      attributes attrs;
      attrs.lg_ = lg;
      attrs.id_ = atom("one");
      context ctx1(attrs);
      attrs.id_ = atom("two");
      context ctx2(attrs);

      threaded_actor base1 = spawn(ctx1);
      threaded_actor base2 = spawn(ctx2);
      gce::bind(base2, "tcp://127.0.0.1:14923");

      spawn(base2, boost::bind(&service_link_ut::echo_service, _arg1), monitored);
      svcid_t echo_svc = make_svcid("two", "echo_svc");

      base1.sleep_for(millisecs(100));

      connect(base1, "two", "tcp://127.0.0.1:14923");

      base1.link(echo_svc);
      for (std::size_t i=0; i<echo_num; ++i)
      {
        base1->send(echo_svc, "echo");
        base1->match("echo").guard(echo_svc).recv();
      }

      spawn(base1, boost::bind(&service_link_ut::other_service, _arg1), monitored);
      base1->recv(exit);
      base1->recv(exit);

      base2->recv(exit);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  static void other_service(stackful_actor self)
  {
    log::logger_t& lg = self.get_context().get_logger();
    try
    {
      register_service(self, "other_svc");
      size_t const echo_num = 10;
      svcid_t echo_svc = make_svcid("two", "echo_svc");

      self.monitor(echo_svc);
      for (std::size_t i=0; i<echo_num; ++i)
      {
        self->send(echo_svc, "echo");
        self->match("echo").guard(echo_svc).recv();
      }

      self->send(echo_svc, "end");
      self->recv(exit);
      deregister_service(self, "other_svc");
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "other except: " << ex.what();
    }
  }

  static void echo_service(stackful_actor self)
  {
    log::logger_t& lg = self.get_context().get_logger();
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
      GCE_ERROR(lg) << "echo except: " << ex.what();
    }
  }
};
}
