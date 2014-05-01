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
static std::size_t const msg_size = 10000000;
public:
  static void run()
  {
    std::cout << "actor_pingpong_ut begin." << std::endl;
    test();
    std::cout << "actor_pingpong_ut end." << std::endl;
  }

private:
  static void my_child(self_t self, aid_t sire, aid_t base_id)
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

  static void my_actor(self_t self, aid_t base_id)
  {
    aid_t aid =
      spawn(
        self,
        boost::bind(
          &actor_pingpong_ut::my_child,
          _1, self.get_aid(), base_id
          )
        );

    message m(1);
    for (std::size_t i=0; i<msg_size; ++i)
    {
      self.send(aid, m);
      self.recv(m);
    }
    send(self, aid, 2);
  }

  static void test()
  {
    try
    {
      attributes attrs;
      attrs.thread_num_ = 2;
      //attrs.pack_pool_reserve_size_ = 1000000;
      //attrs.pack_pool_cache_size_ = size_nil;
      context ctx(attrs);

      mixin_t base = spawn(ctx);
      aid_t base_id = base.get_aid();
      aid_t aid =
        spawn(
          base,
          boost::bind(
            &actor_pingpong_ut::my_actor, _1,
            base_id
            )
          );

      boost::timer::auto_cpu_timer t;
      recv(base);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

