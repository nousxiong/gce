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
class services_ut
{
public:
  static void run()
  {
    std::cout << "services_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "services_ut end." << std::endl;
  }

private:
  static void test_common()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");
    try
    {
      size_t ctx_num = 5;
      size_t echo_num = 100;
      typedef boost::shared_ptr<context> context_ptr;

      attributes attrs;
      attrs.id_ = atom("router");
      context ctx(attrs);

      std::vector<context_ptr> ctx_list(ctx_num);
      for (size_t i=0; i<ctx_num; ++i)
      {
        attrs.id_ = to_match(i);
        ctx_list[i] = boost::make_shared<context>(attrs);
      }

      threaded_actor base = spawn(ctx);

      std::deque<threaded_actor> base_list(ctx_num);
      for (size_t i=0; i<ctx_num; ++i)
      {
        base_list[i] = spawn(*ctx_list[i]);
      }
      
      netopt_t opt = make_netopt();
      opt.is_router = 1;
      gce::bind(base, "tcp://127.0.0.1:14923", remote_func_list_t(), opt);

      BOOST_FOREACH(threaded_actor& base, base_list)
      {
        spawn(base, boost::bind(&services_ut::echo_service, _arg1), monitored);
      }

      opt.reconn_period = seconds(1);
      BOOST_FOREACH(threaded_actor& base, base_list)
      {
        connect(base, "router", "tcp://127.0.0.1:14923", opt);
      }
      base.sleep_for(millisecs(200));

      threaded_actor& base1 = base_list[0];
      ctxid_t curr = ctxid_nil;
      for (std::size_t i=0; i<echo_num; ++i)
      {
        base1->send("echo_svc", "echo");
        aid_t sender = base1->recv("echo");
        GCE_VERIFY(curr != sender.ctxid_);
        curr = sender.ctxid_;
      }

      for (std::size_t i=0; i<ctx_num; ++i)
      {
        svcid_t svcid = make_svcid(i, "echo_svc");
        threaded_actor& base = base_list[i];
        base->send(svcid, "end");
        base->recv(exit);
      }
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
