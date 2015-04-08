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
      test_common<log::std_logger_st>();
      test_common<log::std_logger_mt>();
      test_common<log::asio_logger>();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "log_ut end." << std::endl;
  }

private:
  template <typename Logger>
  static void test_common()
  {
    try
    {
      Logger my_lg;
      log::logger_t login = boost::bind(&Logger::output, &my_lg, _arg1, "login");
      log::logger_t logout = boost::bind(&Logger::output, &my_lg, _arg1, "logout");
      log::logger_t lg = boost::bind(&Logger::output, &my_lg, _arg1, "");
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
        log::record scp(lg, log::info);
        scp(__FILE__)(__LINE__) << 
          "\nhello! scope begin, arg: " << i << "\n";
        scp << 
          "in scope, arg: " << ++i << ", " << 3.1f << "\n";
        scp << "error: " << ++i << "\n";
        scp << "scope end";
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
