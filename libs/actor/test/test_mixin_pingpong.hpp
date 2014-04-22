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
class mixin_pingpong_ut
{
static std::size_t const msg_size = 100000;
public:
  static void run()
  {
    std::cout << "mixin_pingpong_ut begin." << std::endl;
    test();
    std::cout << "mixin_pingpong_ut end." << std::endl;
  }

private:
  static void pong_actor(mixin_t pong)
  {
    message msg;
    while (true)
    {
      aid_t sender = pong.recv(msg);
      if (msg.get_type() == 2)
      {
        break;
      }
      else
      {
        pong.send(sender, msg);
      }
    }
  }

  static void test()
  {
    try
    {
      attributes attrs;
      attrs.thread_num_ = 2;
      context ctx(attrs);

      mixin_t ping = spawn(ctx);
      mixin_t pong = spawn(ctx);
      aid_t pong_id = pong.get_aid();
      boost::thread thr(
        boost::bind(
          &mixin_pingpong_ut::pong_actor, boost::ref(pong)
          )
        );

      boost::timer::auto_cpu_timer t;
      message m(1);
      for (std::size_t i=0; i<msg_size; ++i)
      {
        ping.send(pong_id, m);
        ping.recv(m);
      }
      send(ping, pong_id, 2);
      thr.join();
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
};
}

