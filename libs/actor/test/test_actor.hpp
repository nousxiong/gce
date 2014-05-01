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
    reply(self, aid);
  }

  static void my_actor(self_t self, aid_t base_id, mixin_t base)
  {
    std::size_t size = 10;
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

    timer_t tmr(self.get_thread()->get_io_service());
    yield_t yield = self.get_yield();
    tmr.expires_from_now(boost::chrono::milliseconds(1));
    tmr.async_wait(yield);

    for (std::size_t i=0; i<size; ++i)
    {
      aid_t aid;
      message msg;
      do
      {
        aid = self.recv(res_list[i], msg, seconds_t(1));
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
    for (std::size_t i=0; i<5; ++i)
    {
      spawn(mix, boost::bind(&actor_ut::my_actor, _1, base_id, boost::ref(mix)));
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t free_actor_num = 10;
      std::size_t user_thr_num = 5;
      std::size_t my_actor_size = free_actor_num + user_thr_num * 5;
      attributes attrs;
      context ctx(attrs);

      mixin_t base = spawn(ctx);
      aid_t base_id = base.get_aid();
      for (std::size_t i=0; i<free_actor_num; ++i)
      {
        spawn(
          base,
          boost::bind(
            &actor_ut::my_actor, _1,
            base_id, boost::ref(base)
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
