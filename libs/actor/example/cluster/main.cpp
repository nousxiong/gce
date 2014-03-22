///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "gate.hpp"
#include "node.hpp"
#include <gce/actor/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <cstdlib>
#include <algorithm>
#include <iostream>

typedef std::pair<gce::ctxid_t, std::string> game_init_t;

void init_node(
  gce::self_t node,
  gce::ctxid_t node_id,
  std::size_t gate_count,
  std::vector<std::size_t> game_list,
  std::vector<game_init_t> all_game_list
  )
{
  BOOST_FOREACH(std::size_t index, game_list)
  {
    game_init_t const& game_init = all_game_list.at(index);
    gce::bind(node, game_init.second);
  }

  BOOST_FOREACH(std::size_t index, game_list)
  {
    game_init_t const& game_init = all_game_list[index];
    all_game_list.erase(
      std::remove(
        all_game_list.begin(),
        all_game_list.end(),
        game_init
        )
      );
  }

  if (gate_count > 0)
  {
    BOOST_FOREACH(game_init_t const& game_init, all_game_list)
    {
      gce::connect(node, game_init.first, game_init.second);
    }
  }
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 4)
    {
      std::cerr <<
        "Usage: <node ctxid> \
        <gate_count> [<cln_ep> ...] \
        <all game_count> [<gate_ep> <node_id> <name> ...] \
        <game_count> [<index at all game list> ...]"
        << std::endl;
      return 0;
    }

    std::vector<std::string> gate_list;
    std::vector<game_init_t> all_game_list;
    app_ctxid_list_t game_svcid_list;
    std::vector<std::size_t> game_list;

    gce::ctxid_t node_id = gce::atom(argv[1]);
    std::string node_ep(argv[2]);

    std::size_t gate_count =
      boost::lexical_cast<std::size_t>(argv[3]);
    std::size_t i = 4;
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

    nod.start(
      boost::bind(
        &init_node, _1, node_id,
        gate_count, game_list, all_game_list
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
