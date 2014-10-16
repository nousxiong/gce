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

#include "master.hpp"
#include <gce/actor/all.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <vector>

class node
{
public:
  node(
    gce::attributes attrs, 
    std::string master_ep, 
    bool is_master, 
    gce::ctxid_t master_node_id, 
    std::vector<app_init_t> const& gate_list,
    std::vector<app_init_t> const& game_list,
    std::vector<router_init_t> const& router_list
    );
  ~node();

  void wait_end();

private:
  void run(gce::actor<gce::stackful>&, gce::svcid_t master);
  void handle_signal(gce::actor<gce::stackful>&);

private:
  gce::context ctx_;
  gce::actor<gce::threaded> base_;
  boost::asio::signal_set sig_;
  boost::scoped_ptr<master> master_;
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_NODE_HPP
