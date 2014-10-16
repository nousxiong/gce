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
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 4)
    {
      std::cerr <<
        "Usage: <node ctxid> <master_ep> <is_master>" << std::endl;
      std::cerr << "// if is_master != 0 " << 
        "<gate count> [<node_id> <cln_ep> <name> ...] \
        <game count> [<node_id> <gate_ep> <name> ...] \
        <router count> [<node_id> <router_ep> ...]"
        << std::endl;
      std::cerr << "// else " << 
        "<master node id>" 
        << std::endl;
      return 0;
    }

    std::vector<app_init_t> gate_list;
    std::vector<app_init_t> game_list;
    std::vector<router_init_t> router_list;

    gce::ctxid_t node_id = gce::atom(argv[1]);
    std::string master_ep = argv[2];
    bool is_master = boost::lexical_cast<bool>(argv[3]);
    gce::ctxid_t master_node_id = node_id;
    if (is_master)
    {
      std::size_t i = 4;
      std::size_t gate_count =
        boost::lexical_cast<std::size_t>(argv[i]);
      ++i;
      for (std::size_t j=0; j<gate_count; i+=3, ++j)
      {
        app_init_t app_init(gce::atom(argv[i]), argv[i+1], gce::atom(argv[i+2]));
        gate_list.push_back(app_init);
      }

      std::size_t game_count =
        boost::lexical_cast<std::size_t>(argv[i]);
      ++i;
      for (std::size_t j=0; j<game_count; i+=3, ++j)
      {
        app_init_t app_init(gce::atom(argv[i]), argv[i+1], gce::atom(argv[i+2]));
        game_list.push_back(app_init);
      }

      std::size_t router_count =
        boost::lexical_cast<std::size_t>(argv[i]);
      ++i;
      for (std::size_t j=0; j<router_count; i+=2, ++j)
      {
        router_init_t init;
        init.first = gce::atom(argv[i]);
        init.second = argv[i+1];
        router_list.push_back(init);
      }
    }
    else
    {
      master_node_id = gce::atom(argv[4]);
    }

    gce::attributes attrs;
    attrs.id_ = node_id;

    node nod(
      attrs, master_ep, is_master, master_node_id, 
      gate_list, game_list, router_list
      );
    nod.wait_end();
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
