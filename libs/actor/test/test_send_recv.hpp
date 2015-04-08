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
class send_recv_ut
{
public:
  static void run()
  {
    std::cout << "send_recv_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "send_recv_ut end." << std::endl;
  }

public:
  static void ping_pong(stackful_actor self)
  {
    try
    {
      aid_t host, partner;
      host = self->recv("prepare", partner);

      int count_down;
      while (true)
      {
        self->recv("ping_pong", count_down);
        if (count_down == 0)
        {
          self->send(partner, "ping_pong", count_down);
          break;
        }
        --count_down;
        self->send(partner, "ping_pong", count_down);
      }
      self->send(host);
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  static void test_base()
  {
    try
    {
      context ctx;
      threaded_actor base = spawn(ctx);

      int const count_down = 10000;
      aid_t ping = spawn(base, boost::bind(&send_recv_ut::ping_pong, _arg1));
      aid_t pong = spawn(base, boost::bind(&send_recv_ut::ping_pong, _arg1));

      base->send(ping, "prepare", pong);
      base->send(pong, "prepare", ping);

      base->send(ping, "ping_pong", count_down);
      for (std::size_t i=0; i<2; ++i)
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
