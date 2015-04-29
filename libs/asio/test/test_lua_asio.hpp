///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/asio/config.hpp>

namespace gce
{
class lua_asio_ut
{
public:
  static void run()
  {
    std::cout << "lua_asio_ut test_base begin." << std::endl;
    test_base();
    std::cout << "lua_asio_ut test_base end." << std::endl;
  }

private:
  static void test_base()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      attributes attrs;
      attrs.lg_ = lg;
      attrs.lua_reg_list_.push_back(
        boost::bind(&asio::make_libasio, _arg1)
        );
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      aid_t base_aid = base.get_aid();
      aid_t aid = spawn(base, "test_lua_asio/root.lua", monitored);
      base->send(aid, "init", base_aid);

      boost::timer::auto_cpu_timer t;
      base->recv(exit);
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << ex.what();
    }
  }
};
}
