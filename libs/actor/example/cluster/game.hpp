///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_GAME_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_GAME_HPP

#include "app.hpp"
#include <gce/actor/all.hpp>

class game
{
public:
  static gce::aid_t start(
    gce::actor<gce::stackful>& sire, 
    gce::match_t svc_name, 
    app_ctxid_list_t game_list
    );

private:
  static void run(
    gce::actor<gce::stackful>& self, 
    gce::match_t svc_name, 
    app_ctxid_list_t game_list
    );
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_GAME_HPP
