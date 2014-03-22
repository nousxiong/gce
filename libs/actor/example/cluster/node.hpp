///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_NODE_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_NODE_HPP

#include "basic_app.hpp"
#include <gce/actor/all.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

class node
{
public:
  explicit node(gce::attributes);
  ~node();

  void wait_end();

public:
  void add_app(basic_app_ptr);
  void start(gce::actor_func_t);

private:
  void run(gce::self_t, gce::actor_func_t);
  void handle_signal(gce::self_t);

private:
  gce::context ctx_;
  gce::mixin_t base_;
  boost::asio::signal_set sig_;

  std::vector<basic_app_ptr> app_list_;
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_NODE_HPP
