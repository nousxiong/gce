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
class actor_pingpong_ut
{
static std::size_t const msg_size = 100000;
public:
  static void run()
  {
    std::cout << "actor_pingpong_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "actor_pingpong_ut end." << std::endl;
  }

private:
  static void my_child(stackful_actor self, aid_t sire, aid_t base_id)
  {
    message msg;
    while (true)
    {
      self.recv(msg);
      if (msg.get_type() == 2)
      {
        break;
      }
      else
      {
        self.send(sire, msg);
      }
    }
    self.send(base_id, msg);
  }

  static void my_actor(stackful_actor self, aid_t base_id)
  {
    aid_t aid =
      spawn(
        self,
        boost::bind(
          &actor_pingpong_ut::my_child,
          _arg1, self.get_aid(), base_id
          )
        );

    message m(1);
    std::size_t const scale = 100;
    for (std::size_t i=0, size=msg_size/scale; i<size; ++i)
    {
      for (std::size_t j=0; j<scale; ++j)
      {
        self.send(aid, m);
      }
      for (std::size_t j=0; j<scale; ++j)
      {
        self.recv(m);
      }
    }
    self->send(aid, 2);
  }

  static void test()
  {
    try
    {
      context ctx;
      threaded_actor base = spawn(ctx);

      aid_t base_id = base.get_aid();
      aid_t aid =
        spawn(
          base,
          boost::bind(
            &actor_pingpong_ut::my_actor, _arg1,
            base_id
            )
          );

      boost::timer::auto_cpu_timer t;
      base->recv();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

