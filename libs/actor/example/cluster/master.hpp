///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_MASTER_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_MASTER_HPP

#include "app.hpp"
#include <gce/actor/all.hpp>
#include <vector>
#include <string>

class master
{
public:
  master(
    std::vector<app_init_t> const& gate_list,
    std::vector<app_init_t> const& game_list,
    std::vector<router_init_t> const& router_list
    );
  ~master();

public:
  gce::aid_t start(gce::actor<gce::stackful>& sire);

private:
  void run(gce::actor<gce::stackful>&);

private:
  std::vector<app_init_t> const gate_list_;
  std::vector<app_init_t> const game_list_;
  std::vector<router_init_t> const router_list_;
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_MASTER_HPP
