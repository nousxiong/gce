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
class log_ut
{
public:
  static void run()
  {
    std::cout << "log_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "log_ut end." << std::endl;
  }

private:
  static void test_common()
  {
    try
    {
      log::asio_logger my_lg;
      log::logger_t login = boost::bind(&log::asio_logger::output, &my_lg, _1, "login");
      log::logger_t logout = boost::bind(&log::asio_logger::output, &my_lg, _1, "logout");
      log::logger_t lg = boost::bind(&log::asio_logger::output, &my_lg, _1, "");
      log::logger_t empty_lg;

      int i = 1;
      GCE_INFO(login)(__FILE__)(__LINE__) << 
        "hello! new user login, arg: " << i;
      GCE_INFO(logout)(__FILE__) << 
        "user logout, arg: " << ++i << ", " << 3.1f;
      GCE_ERROR(lg) << "error: " << ++i;
      GCE_INFO(empty_lg) << "this shouldn't been printed\n";

      {
        // test scope log
        log::scope scp(lg);
        GCE_INFO(scp)(__FILE__)(__LINE__) << 
          "hello! scope begin, arg: " << i;
        GCE_INFO(scp)(__FILE__) << 
          "in scope, arg: " << ++i << ", " << 3.1f;
        GCE_ERROR(scp) << "error: " << ++i;
        GCE_INFO(scp) << "scope end";
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
