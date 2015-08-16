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
class service_broken_ut
{
public:
  static void run()
  {
    std::cout << "service_broken_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "service_broken_ut end." << std::endl;
  }

private:
  static void test_common()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");
    try
    {
      size_t const echo_num = 3;
      attributes attrs;
      attrs.lg_ = lg;
      attrs.id_ = atom("one");
      //attrs.thread_num_ = 1;
      context ctx1(attrs);
      attrs.id_ = atom("two");
      context ctx2(attrs);

      threaded_actor base1 = spawn(ctx1);
      threaded_actor base2 = spawn(ctx2);
      gce::bind(base2, "tcp://127.0.0.1:14923");

      spawn(base2, boost::bind(&service_broken_ut::echo_service, _arg1), monitored);
      svcid_t echo_svc = make_svcid("two", "echo_svc");

      base1.sleep_for(millisecs(100));

      netopt_t opt = make_netopt();
      opt.reconn_wait_period = millisecs(500);
      opt.heartbeat_period = seconds(1);
      connect(base1, "two", "tcp://127.0.0.1:14923", opt);

      broken(base1, echo_num);

      spawn(base1, boost::bind(&service_broken_ut::other_service, _arg1), monitored);
      base1->recv(exit);

      base1->send(echo_svc, "end");
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
      size_t const echo_num = 3;
      svcid_t echo_svc = make_svcid("two", "echo_svc");

      broken(self, echo_num, false);

      deregister_service(self, "other_svc");
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "other except: " << ex.what();
    }
  }

  template <typename Actor>
  static void broken(Actor& a, size_t const echo_num, bool is_link = true)
  {
    svcid_t echo_svc = make_svcid("two", "echo_svc");
    if (is_link) a.link(echo_svc); else a.monitor(echo_svc);
    a->send(echo_svc, "echo");
    for (std::size_t i=0; i<echo_num; )
    {
      errcode_t ec;
      a->match("echo").guard(echo_svc, ec).recv();
      if (ec)
      {
        std::cout << "broken\n";
        if (is_link) a.link(echo_svc); else a.monitor(echo_svc);
        ++i;
      }
      else
      {
        a.sleep_for(millisecs(3200));
        a->send(echo_svc, "echo");
      }
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
        else if (type == atom("end"))
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
