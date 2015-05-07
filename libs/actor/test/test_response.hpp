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
class response_ut
{
public:
  static void run()
  {
    std::cout << "response_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "response_ut end." << std::endl;
  }

private:
  static void my_child(stackful_actor self)
  {
    aid_t aid = self->recv();
    self->send(aid, "ret");
  }

  static void my_actor(stackful_actor self, aid_t base_id)
  {
    //try
    {
      std::size_t size = 50;
      std::vector<resp_t> res_list(size);
      for (std::size_t i=0; i<size; ++i)
      {
        aid_t aid =
          spawn(
            self,
            boost::bind(&response_ut::my_child, _arg1),
            monitored
            );
        res_list[i] = self->request(aid);
        message msg;
        pattern patt;
        patt.add_match("ret");
        self.recv(msg, patt);
      }

      timer_t tmr(self.get_context().get_io_service());
      yield_t yield = self.get_yield();
      tmr.expires_from_now(boost::chrono::milliseconds(1));
      tmr.async_wait(yield);

      for (std::size_t i=0; i<size; ++i)
      {
        aid_t aid;
        message msg;
        do
        {
          aid = self.respond(res_list[i], msg, seconds(1));
        }
        while (aid == aid_nil);
      }

      tmr.expires_from_now(boost::chrono::milliseconds(1));
      tmr.async_wait(yield);

      self->send(base_id);
    }
    /*catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }*/
  }

  static void my_thr(context& ctx, aid_t base_id)
  {
    threaded_actor a = spawn(ctx);
    for (std::size_t i=0; i<2; ++i)
    {
      spawn(a, boost::bind(&response_ut::my_actor, _arg1, base_id));
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
      context ctx(attrs);
      threaded_actor base = spawn(ctx);

      aid_t base_id = base.get_aid();
      for (std::size_t i=0; i<free_actor_num; ++i)
      {
        spawn(
          base,
          boost::bind(
            &response_ut::my_actor, _arg1,
            base_id
            )
          );
      }

      boost::thread_group thrs;
      for (std::size_t i=0; i<user_thr_num; ++i)
      {
        thrs.create_thread(
          boost::bind(
            &response_ut::my_thr,
            boost::ref(ctx), base_id
            )
          );
      }

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
};
}
