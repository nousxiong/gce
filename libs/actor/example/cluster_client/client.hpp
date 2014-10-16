///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_CLIENT_CLIENT_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_CLIENT_CLIENT_HPP

#include "socket.hpp"
#include <gce/actor/all.hpp>
#include <boost/array.hpp>
#include <string>

class client
{
public:
  explicit client(std::string);
  ~client();

public:
  void run();

private:
  void command(std::string cmd, gce::aid_t);
  boost::array<std::string, 2> parse_potocol(std::string);
  void pri_run(gce::actor<gce::stackful>&);
  void recv(gce::actor<gce::stackful>&, tcp_socket&);
  gce::attributes get_attrs();

private:
  boost::array<std::string, 2> gate_ep_;
  gce::context ctx_;
  gce::actor<gce::threaded> base_;

  std::string username_;
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_CLIENT_CLIENT_HPP
