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
class assert_ut
{
public:
  static void run()
  {
    std::cout << "assert_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "assert_ut end." << std::endl;
  }

private:
  static void test_common()
  {
    {
      log::std_logger_st my_lg;
      log::logger_t lg = boost::bind(&log::std_logger_st::output, &my_lg, _arg1, "");

      try
      {
        int i = 0;
        std::string str("std::cerr");
        GCE_ASSERT(i == 0 && str == "hi")(i)(str).log().except();
#ifdef GCE_ENABLE_ASSERT
        BOOST_ASSERT_MSG(false, "shouldn't be here");
#endif
      }
      catch (std::exception& ex)
      {
        std::cerr << ex.what() << std::endl;
      }

      std::cout << "------------------" << std::endl;

      try
      {
        int i = 0;
        std::string str("verify");
        GCE_VERIFY(i == 0 && str == "hi")(i)(str).except<std::runtime_error>();
        BOOST_ASSERT_MSG(false, "shouldn't be here");
      }
      catch (std::runtime_error& ex)
      {
        std::cerr << ex.what() << std::endl;
      }

      std::cout << "------------------" << std::endl;

      try
      {
        GCE_VERIFY(false).msg("std::runtime_error").except<std::runtime_error>();
        BOOST_ASSERT_MSG(false, "shouldn't be here");
      }
      catch (std::runtime_error& ex)
      {
        std::cerr << ex.what() << std::endl;
      }

      std::cout << "------------------" << std::endl;

      try
      {
        int i = 0;
        std::string str("std_logger");
        GCE_ASSERT(i == 0 && str == "hi")(i)(str)
          .log(lg).except();
#ifdef GCE_ENABLE_ASSERT
        BOOST_ASSERT_MSG(false, "shouldn't be here");
#endif
      }
      catch (std::exception& ex)
      {
        GCE_ERROR(lg) << ex.what();
      }
    }

    /*int i = 0;
    std::string str("default");
    GCE_ASSERT(i == 0 && str == "hi")(i)(str);
#ifdef GCE_ENABLE_ASSERT
    BOOST_ASSERT_MSG(false, "shoudn't be here");
#endif*/
  }
};
}
