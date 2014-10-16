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

void echo(gce::actor<gce::stackful>& self)
{
  /// wait for "start" message. 
  /// if and only if after fetch "start", then others
  gce::recv(self, gce::atom("start"));
  std::cout << "start!" << std::endl;

  /// handle other messages
  gce::message msg;
  gce::aid_t sender = self.recv(msg);
  std::cout << "recv message: " << gce::atom(msg.get_type()) << std::endl;

  /// reply
  gce::send(self, sender);
}

int main()
{
  gce::context ctx;

  /// spawn a thread_mapped_actor
  gce::actor<gce::threaded> base = gce::spawn(ctx);

  gce::aid_t echo_actor = gce::spawn(base, boost::bind(&echo, _1));

  /// send "hi" message to echo
  gce::send(base, echo_actor, gce::atom("hi"));

  /// send "start" message to echo, after "hi" message
  gce::send(base, echo_actor, gce::atom("start"));

  /// ... and wait for a response
  gce::recv(base);

  return 0;
}
