///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "gate.hpp"
#include "game.hpp"
#include "node.hpp"
#include <gce/actor/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <iostream>

typedef std::pair<gce::ctxid_t, std::string> app_init_t;

void init_node(
  gce::self_t node,
  std::size_t gate_count,
  std::vector<std::size_t> game_list,
  std::vector<app_init_t> all_game_list,
  std::vector<std::size_t> router_list,
  std::vector<app_init_t> all_router_list
  )
{
  BOOST_FOREACH(std::size_t index, router_list)
  {
    app_init_t const& app_init = all_router_list.at(index);
    gce::bind(node, app_init.second, true);
  }

  BOOST_FOREACH(std::size_t index, game_list)
  {
    app_init_t const& app_init = all_game_list.at(index);
    gce::bind(node, app_init.second);
  }

  BOOST_FOREACH(std::size_t index, game_list)
  {
    app_init_t const& app_init = all_game_list[index];
    all_game_list.erase(
      std::remove(
        all_game_list.begin(),
        all_game_list.end(),
        app_init
        )
      );
  }

  if (gate_count > 0)
  {
    BOOST_FOREACH(app_init_t const& app_init, all_game_list)
    {
      gce::connect(node, app_init.first, app_init.second);
    }
  }

  BOOST_FOREACH(app_init_t const& app_init, all_router_list)
  {
    gce::connect(node, app_init.first, app_init.second, true);
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 6)
    {
      std::cerr <<
        "Usage: <node ctxid> \
        <gate_count> [<cln_ep> ...] \
        <all game_count> [<gate_ep> <node_id> <name> ...] \
        <game_count> [<index at all game list> ...] \
        <all router count> [<router_ep> <node_id> ...] \
        <router count> [<index at all router list> ...]"
        << std::endl;
      return 0;
    }

    std::vector<std::string> gate_list;
    std::vector<app_init_t> all_game_list, all_router_list;
    app_ctxid_list_t game_svcid_list;
    std::vector<std::size_t> game_list, router_list;

    gce::ctxid_t node_id = gce::atom(argv[1]);

    std::size_t gate_count =
      boost::lexical_cast<std::size_t>(argv[2]);
    std::size_t i = 3;
    for (std::size_t j=0; j<gate_count; ++i, ++j)
    {
      gate_list.push_back(argv[i]);
    }

    std::size_t all_game_count =
      boost::lexical_cast<std::size_t>(argv[i]);
    ++i;
    for (std::size_t j=0; j<all_game_count; i+=3, ++j)
    {
      char const* gate_ep = argv[i];
      char const* ctxid = argv[i+1];
      char const* name = argv[i+2];
      game_svcid_list.push_back(
        gce::svcid_t(
          gce::atom(ctxid),
          gce::atom(name)
          )
        );
      all_game_list.push_back(
        std::make_pair(gce::atom(ctxid), gate_ep)
        );
    }

    std::size_t game_count =
      boost::lexical_cast<std::size_t>(argv[i]);
    ++i;
    for (std::size_t j=0; j<game_count; ++i, ++j)
    {
      std::size_t index = boost::lexical_cast<std::size_t>(argv[i]);
      game_list.push_back(index);
    }

    std::size_t all_router_count =
      boost::lexical_cast<std::size_t>(argv[i]);
    ++i;
    for (std::size_t j=0; j<all_router_count; i+=2, ++j)
    {
      char const* router_ep = argv[i];
      char const* ctxid = argv[i+1];
      all_router_list.push_back(
        std::make_pair(gce::atom(ctxid), router_ep)
        );
    }

    std::size_t router_count =
      boost::lexical_cast<std::size_t>(argv[i]);
    ++i;
    for (std::size_t j=0; j<router_count; ++i, ++j)
    {
      std::size_t index = boost::lexical_cast<std::size_t>(argv[i]);
      router_list.push_back(index);
    }

    gce::attributes attrs;
    attrs.id_ = node_id;

    node nod(attrs);

    BOOST_FOREACH(std::string cln_ep, gate_list)
    {
      nod.add_app(
        basic_app_ptr(
          new gate(cln_ep, game_svcid_list)
          )
        );
    }

    BOOST_FOREACH(std::size_t index, game_list)
    {
      gce::svcid_t svc = game_svcid_list.at(index);
      nod.add_app(
        basic_app_ptr(
          new game(svc.name_, game_svcid_list)
          )
        );
    }

    nod.start(
      boost::bind(
        &init_node, _1, gate_count,
        game_list, all_game_list,
        router_list, all_router_list
        )
      );
    nod.wait_end();
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
