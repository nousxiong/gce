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
#include <vector>
#include <string>

typedef std::vector<gce::svcid_t> app_ctxid_list_t;
///----------------------------------------------------------------------------
gce::svcid_t select_game_app(
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
struct app_init
{
  app_init(gce::ctxid_t node_id, std::string ep, gce::match_t name)
    : node_id_(node_id)
    , ep_(ep)
    , name_(name)
  {
  }

  gce::ctxid_t node_id_;
  std::string ep_;
  gce::match_t name_;
};
///----------------------------------------------------------------------------
struct game_ep_info
{
  gce::ctxid_t node_id_;
  std::string ep_;
};
GCE_PACK(game_ep_info, (node_id_)(ep_));
///----------------------------------------------------------------------------
struct gate_info
{
  gce::match_t svc_name_;
  std::string ep_;
  app_ctxid_list_t game_list_;
};
GCE_PACK(gate_info, (svc_name_)(ep_)(game_list_));
///----------------------------------------------------------------------------
struct game_info
{
  gce::match_t svc_name_;
  std::string ep_;
  app_ctxid_list_t game_list_;
};
GCE_PACK(game_info, (svc_name_)(ep_)(game_list_));
///----------------------------------------------------------------------------
typedef app_init app_init_t;
struct router_init
{
  gce::ctxid_t first;
  std::string second;
};
typedef router_init router_init_t;
GCE_PACK(router_init, (first)(second));
///----------------------------------------------------------------------------
struct node_info
{
  std::vector<gate_info> gate_list_;
  std::vector<game_info> game_list_;
  std::vector<router_init_t> router_list_;
};
GCE_PACK(node_info, (gate_list_)(game_list_)(router_list_));
///----------------------------------------------------------------------------

#endif /// GCE_ACTOR_EXAMPLE_CLUSTER_APP_HPP
