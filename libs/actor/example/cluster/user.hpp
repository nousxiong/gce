///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_USER_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_USER_HPP

#include "app.hpp"
#include <gce/actor/all.hpp>
#include <boost/shared_ptr.hpp>

typedef gce::aid_t cid_t;
class user
{
public:
  static void run(
    gce::actor<gce::stackful>&, app_ctxid_list_t game_list,
    gce::aid_t old_usr_aid, gce::aid_t master,
    cid_t cid, std::string username, std::string passwd
    );
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_USER_HPP
