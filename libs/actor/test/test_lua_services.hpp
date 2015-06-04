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
class lua_services_ut
{
public:
  static void run()
  {
    std::cout << "lua lua_services_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "lua lua_services_ut end." << std::endl;
  }

private:
  static void test_base()
  {
    try
    {
      std::size_t echo_num = 10;

      gce::log::asio_logger lg;
      attributes attrs;
      attrs.lg_ = boost::bind(&gce::log::asio_logger::output, &lg, _arg1, "");
      
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

      spawn(base1, "test_lua_actor/services.lua", monitored);
      spawn(base2, "test_lua_actor/services.lua", monitored);

      opt.reconn_period = seconds(1);
      connect(base1, "router", "tcp://127.0.0.1:14923", opt);
      connect(base2, "router", "tcp://127.0.0.1:14923", opt);
      base2.sleep_for(millisecs(200));

      for (std::size_t i=0; i<echo_num; ++i)
      {
        base1->send("echo_svc", "echo");
        base1->recv("echo");
      }

      base1->send(make_svcid("one", "echo_svc"), "end");
      base1->send(make_svcid("two", "echo_svc"), "end");

      base1->recv(exit);
      base2->recv(exit);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
