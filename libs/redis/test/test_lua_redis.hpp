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
class lua_redis_ut
{
public:
  static void run()
  {
    std::cout << "lua_redis_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "lua_redis_ut end." << std::endl;
  }

private:
  static void test_common()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      attributes attr;
      attr.lg_ = lg;
      attr.lua_reg_list_.push_back(
        boost::bind(&asio::make_libasio, _arg1)
        );
      attr.lua_reg_list_.push_back(
        boost::bind(&redis::make_libredis, _arg1)
        );
      context ctx(attr);
      threaded_actor base = spawn(ctx);
      aid_t base_aid = base.get_aid();
      std::string errmsg;

      redis::context rctx(ctx);
      redis::ctxid_t redis_ctxid = rctx.get_ctxid();

      {
        boost::timer::auto_cpu_timer t;
        aid_t aid = spawn(base, "test_lua_redis/redis_ut.lua", monitored);
        base->send(aid, "init", redis_ctxid);

        exit_code_t exc;
        base->match(exit).recv(exc, errmsg);
        if (exc != exit_normal)
        {
          GCE_ERROR(lg) << errmsg;
        }
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "test_common except: " << ex.what();
    }
  }
};
}
