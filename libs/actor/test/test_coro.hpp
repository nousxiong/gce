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
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_common();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "coro_ut end." << std::endl;
  }

private:
  static void my_actor(stackful_actor self, aid_t base)
  {
    std::size_t loop_num = 10;
    yield_t yield = self.get_yield();
    timer_t tmr(self.get_context().get_io_service());

    for (std::size_t i=0; i<loop_num; ++i)
    {
      self->send(base, "echo");
      self->recv("echo");

      tmr.expires_from_now(boost::chrono::milliseconds(1));
      tmr.async_wait(yield);
    }
  }

  static void test_common()
  {
    try
    {
      std::size_t actor_num = 10;
      context ctx;
      threaded_actor base = spawn(ctx);

      for (std::size_t a=0; a<5; ++a)
      {
        for (std::size_t i=0; i<actor_num; ++i)
        {
          aid_t aid = spawn(
            base,
            boost::bind(&coro_ut::my_actor, _arg1, base.get_aid()),
            monitored
            );
        }

        std::size_t i=0;
        while (i < actor_num)
        {
          message msg;
          aid_t sender = base.recv(msg);
          match_t type = msg.get_type();
          if (type == atom("echo"))
          {
            base.send(sender, msg);
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

