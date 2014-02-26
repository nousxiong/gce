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
class actor_ut
{
public:
  static void run()
  {
    std::cout << "actor_ut begin." << std::endl;
    test_common();
    std::cout << "actor_ut end." << std::endl;
  }

private:
  static void my_child(self_t self)
  {
    aid_t aid = recv(self);
    //wait(self, seconds_t(3));
    reply(self, aid);
  }

  static void my_actor(self_t self, aid_t base_id)
  {
    std::size_t size = 50;
    std::vector<response_t> res_list(size);
    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid =
        spawn(
          self,
          boost::bind(&actor_ut::my_child, _1)
          );
      res_list[i] = request(self, aid);
    }

    timer_t tmr(self.get_cache_pool()->get_context().get_io_service());
    yield_t yield = self.get_yield();
    tmr.expires_from_now(boost::chrono::milliseconds(1));
    tmr.async_wait(yield);

    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid;
      do
      {
        aid = recv(self, res_list[i], seconds_t(1));
      }
      while (!aid);
    }

    tmr.expires_from_now(boost::chrono::milliseconds(1));
    tmr.async_wait(yield);

    send(self, base_id);
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    mixin_t mix = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn(mix, boost::bind(&actor_ut::my_actor, _1, base_id));
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
      attrs.mixin_num_ = user_thr_num + 1;
      context ctx(attrs);

      mixin_t base = spawn(ctx);
      aid_t base_id = base.get_aid();
      for (std::size_t i=0; i<free_actor_num; ++i)
      {
        spawn(
          base,
          boost::bind(
            &actor_ut::my_actor, _1,
            base_id
            )
          );
      }

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &actor_ut::my_thr,
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
