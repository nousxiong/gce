///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifdef GCE_LUA
#include <gce/actor/config.hpp>

namespace gce
{
class lua_actor_ut
{
public:
  static void run()
  {
    std::cout << "lua_actor_ut begin." << std::endl;
    test_common();
    std::cout << "lua_actor_ut end." << std::endl;
  }

private:
  static void my_thr(context& ctx, aid_t base_id)
  {
    actor<threaded> a = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      aid_t aid = spawn(a, std::string("test_lua_actor/my_actor.lua"));
      send(a, aid, atom("init"), base_id);
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t free_actor_num = 20;
      std::size_t user_thr_num = 0;
      std::size_t my_actor_size = free_actor_num + user_thr_num * 2;
      attributes attrs;
      attrs.lua_gce_path_list_.push_back(".");
      attrs.lua_gce_path_list_.push_back("..");
      context ctx(attrs);
      actor<threaded> base = spawn(ctx);

      aid_t base_id = base.get_aid();
      for (std::size_t i=0; i<free_actor_num; ++i)
      {
        aid_t aid = spawn(base, std::string("test_lua_actor/my_actor.lua"));
        send(base, aid, atom("init"), base_id);
      }
      
      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &lua_actor_ut::my_thr,
            boost::ref(ctx), base_id
            )
          );
      }

      thrs.join_all();

      for (std::size_t i=0; i<my_actor_size; ++i)
      {
        recv(base);
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}
#endif
