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
class reconn_ut
{
public:
  static void run()
  {
    std::cout << "reconn_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "reconn_ut end." << std::endl;
  }

private:
  static void service(stackful_actor self)
  {
    log::logger_t& lg = self.get_context().get_logger();
    register_service(self, "echo_svc");
    while (true)
    {
      message msg;
      aid_t sender = self.recv(msg);
      match_t type = msg.get_type();
      //GCE_INFO(lg) << atom(type);
      if (atom(type) == "login")
      {
        self->send(sender, "login_ret");
      }
      else
      {
        break;
      }
    }
    deregister_service(self, "echo_svc");
  }

  static void server(size_t const reset_num, log::logger_t lg)
  {
    for (size_t i=0; i<reset_num; ++i)
    {
      attributes attr;
      attr.id_ = atom("svr");
      attr.lg_ = lg;
      context ctx(attr);
      threaded_actor base = spawn(ctx);
      gce::bind(base, "tcp://127.0.0.1:23333", remote_func_list_t());
      spawn(base, boost::bind(&reconn_ut::service, _arg1), monitored);
      base->recv(exit);
    }
  }

  static void test_common()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");
    try
    {
      size_t const reconn_num = 5;
      attributes attr;
      attr.lg_ = lg;
      attr.id_ = atom("cln");
      context ctx(attr);
      threaded_actor base = spawn(ctx);
      boost::thread thr(boost::bind(&reconn_ut::server, reconn_num, lg));
      base.sleep_for(seconds(1));

      netopt_t opt = make_netopt();
      opt.heartbeat_period = seconds(2);
      opt.heartbeat_count = 2;
      opt.reconn_period = seconds(1);
      opt.reconn_try = 100;
      connect(base, "svr", "tcp://127.0.0.1:23333", opt);

      svcid_t svc = make_svcid("svr","echo_svc");
      base->send(svc, "login");
      aid_t svc_aid = base->recv("login_ret");

      for (size_t i=0; i<reconn_num; ++i)
      {
        while (true)
        {
          base->send(svc, "login");
          errcode_t ec;
          aid_t sender = base->match("login_ret").guard(svc_aid, ec).recv();
          if (!ec)
          {
            svc_aid = sender;
            base.monitor(svc_aid);
            base->send(svc, "disconn");
            base->recv(exit);
            std::cout << "exit ok.\n";
            break;
          }
        }
      }

      thr.join();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
