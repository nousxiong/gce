///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/config.hpp>

namespace gce
{
class lua_actor_ut
{
public:
  static void run()
  {
    std::cout << "lua_actor_ut test_base begin." << std::endl;
    test_base();
    std::cout << "lua_actor_ut test_base end." << std::endl;

    std::cout << "lua_actor_ut test_common begin." << std::endl;
    test_common();
    std::cout << "lua_actor_ut test_common end." << std::endl;
  }

private:
  static void my_thr(context& ctx, aid_t base_aid)
  {
    threaded_actor a = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      aid_t aid = spawn(a, "test_lua_actor/my_actor.lua");
      a->send(aid, "init", base_aid);
    }
  }

  static void native_actor(stackful_actor self)
  {
    try
    {
      aid_t sender = self->recv();
      self->reply(sender);
    }
    catch (std::exception& ex)
    {
      std::cerr << "native_actor except: " << ex.what() << std::endl;
    }
  }

  static void test_base()
  {
    try
    {
      gce::log::asio_logger lg;
      context::init_t init;
      init.attrs_.lg_ = boost::bind(&gce::log::asio_logger::output, &lg, _arg1, "");
      init.add_native_func<stackful>("my_native", boost::bind(&lua_actor_ut::native_actor, _arg1));
      std::size_t free_actor_num = 10;
      std::size_t user_thr_num = 5;
      std::size_t my_actor_size = free_actor_num + user_thr_num * 2;
      context ctx(init);
      threaded_actor base = spawn(ctx);

      aid_t base_aid = base.get_aid();
      for (std::size_t i=0; i<free_actor_num; ++i)
      {
        aid_t aid = spawn(base, "test_lua_actor/my_actor.lua");
        base->send(aid, "init", base_aid);
      }
      
      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &lua_actor_ut::my_thr,
            boost::ref(ctx), base_aid
            )
          );
      }

      boost::timer::auto_cpu_timer t;
      thrs.join_all();

      for (std::size_t i=0; i<my_actor_size; ++i)
      {
        base->recv();
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  static void test_common()
  {
    try
    {
      context ctx;
      threaded_actor base = spawn(ctx);

      aid_t base_aid = base.get_aid();
      aid_t aid = spawn(base, "test_lua_actor/root.lua", monitored);
      base->send(aid, "init", base_aid);

      boost::timer::auto_cpu_timer t;
      base->recv(exit);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
