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

void quiter(gce::self_t self)
{
  /// wait for gce::exit from link actor
  gce::recv(self);
}

void link(gce::self_t self)
{
  /// create 10 actor and link with them
  for (std::size_t i=0; i<10; ++i)
  {
    gce::spawn(self, boost::bind(&quiter, _1), gce::linked);
  }

  /// quit, will send 10 gce::exit to quiter actors 
  /// and 1 gce::exit to base actor(in main)
}

int main()
{
  gce::context ctx;

  /// create a link actor and monitor it.
  gce::spawn(ctx, boost::bind(&link, _1), gce::monitored);

  /// wait for gce::exit message
  gce::recv(ctx);

  std::cout << "end" << std::endl;

  return 0;
}
