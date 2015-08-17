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
class lua_service_link_ut
{
public:
  static void run()
  {
    std::cout << "lua_service_link_ut begin." << std::endl;
    test_common();
    std::cout << "lua_service_link_ut end." << std::endl;
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

      spawn(base2, "test_lua_actor/service_link_echo.lua", monitored);
      svcid_t echo_svc = make_svcid("two", "echo_svc");

      base1.sleep_for(millisecs(100));

      connect(base1, "two", "tcp://127.0.0.1:14923");

      base1.link(echo_svc);
      for (std::size_t i=0; i<echo_num; ++i)
      {
        base1->send(echo_svc, "echo");
        base1->match("echo").guard(echo_svc).recv();
      }

      spawn(base1, "test_lua_actor/service_link_other.lua", monitored);
      base1->recv(exit);
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
