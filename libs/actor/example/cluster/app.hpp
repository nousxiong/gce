///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXAMPLE_CLUSTER_APP_HPP
#define GCE_ACTOR_EXAMPLE_CLUSTER_APP_HPP

#include "hash.hpp"
#include <gce/actor/all.hpp>

typedef std::vector<gce::svcid_t> app_ctxid_list_t;
///----------------------------------------------------------------------------
inline gce::svcid_t select_game_app(
  app_ctxid_list_t const& game_list,
  std::string const& username
  )
{
  hash_t h = hash(username.data(), username.size());
  gce::match_t i = h % game_list.size();
  BOOST_ASSERT(i <= game_list.size());
  return game_list[i];
}
///----------------------------------------------------------------------------

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_APP_HPP
