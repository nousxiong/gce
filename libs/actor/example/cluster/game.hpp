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
#include "basic_app.hpp"
#include <gce/actor/all.hpp>

class game
  : public basic_app
{
public:
  game(gce::match_t svc_name, app_ctxid_list_t game_list);
  ~game();

public:
  gce::aid_t start(gce::self_t sire);

private:
  void run(gce::self_t);

private:
  gce::match_t const svc_name_;
  app_ctxid_list_t const game_list_;
};

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_GAME_HPP
