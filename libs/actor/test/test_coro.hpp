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
class coro_ut
{
public:
  static void run()
  {
    std::cout << "coro_ut begin." << std::endl;
    test_common();
    std::cout << "coro_ut end." << std::endl;
  }

private:
  static void my_actor(actor<stacked>& self, aid_t base)
  {
    std::size_t loop_num = 10;
    yield_t yield = self.get_yield();
    timer_t tmr(self.get_cache_pool()->get_context().get_io_service());

    for (std::size_t i=0; i<loop_num; ++i)
    {
      send(self, base, atom("echo"));
      recv(self, atom("echo"));

      tmr.expires_from_now(boost::chrono::milliseconds(1));
      tmr.async_wait(yield);
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t actor_num = 100;
      context ctx;

      for (std::size_t a=0; a<5; ++a)
      {
        for (std::size_t i=0; i<actor_num; ++i)
        {
          aid_t aid = spawn(
            ctx,
            boost::bind(&coro_ut::my_actor, _1, ctx.get_aid()),
            monitored
            );
        }

        std::size_t i=0;
        while (i < actor_num)
        {
          message msg;
          aid_t sender = ctx.recv(msg);
          match_t type = msg.get_type();
          if (type == atom("echo"))
          {
            ctx.send(sender, msg);
          }
          else
          {
            ++i;
          }
        }
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

