///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/all.hpp>
#include <iostream>
#include <string>
#include <cstdlib>

void pingpong(gce::actor<gce::stackful>& self)
{
  try
  {
    gce::aid_t partner;
    gce::aid_t host = gce::recv(self, gce::atom("prepare"), partner);

    int count_down;
    while (true)
    {
      gce::recv(self, gce::atom("pingpong"), count_down);
      if (count_down == 0)
      {
        break;
      }
      --count_down;
      gce::send(self, partner, gce::atom("pingpong"), count_down);
    }
    gce::send(self, host, gce::atom("bye"));
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
}

int main(int argc, char* argv[])
{
  try
  {
    int const count_down =
      (argc > 1 && std::atoi(argv[1]) > 0) ? std::atoi(argv[1]) : 10000;

    gce::context ctx;
    gce::actor<gce::threaded> base = gce::spawn(ctx);
    gce::aid_t ping = gce::spawn(base, boost::bind(&pingpong, _1));
    gce::aid_t pong = gce::spawn(base, boost::bind(&pingpong, _1));

    gce::send(base, ping, gce::atom("prepare"), pong);
    gce::send(base, pong, gce::atom("prepare"), ping);

    gce::send(base, ping, gce::atom("pingpong"), count_down);
    gce::recv(base, gce::atom("bye"));
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  return 0;
}
