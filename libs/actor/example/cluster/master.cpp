///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "master.hpp"
#include <boost/foreach.hpp>
#include <map>

///----------------------------------------------------------------------------
master::master(
  std::vector<app_init_t> const& gate_list,
  std::vector<app_init_t> const& game_list,
  std::vector<router_init_t> const& router_list
  )
  : gate_list_(gate_list)
  , game_list_(game_list)
  , router_list_(router_list)
{
}
///----------------------------------------------------------------------------
master::~master()
{
}
///----------------------------------------------------------------------------
gce::aid_t master::start(gce::actor<gce::stackful>& sire)
{
  return 
    gce::spawn(
      sire, 
      boost::bind(&master::run, this, _1), 
      gce::monitored
      );
}
///----------------------------------------------------------------------------
void master::run(gce::actor<gce::stackful>& self)
{
  try
  {
    gce::register_service(self, gce::atom("master"));
    gce::aid_t sire = gce::recv(self, gce::atom("init"));
    gce::reply(self, sire);

    /// sort apps info using node ctxid key
    typedef std::map<gce::svcid_t, node_info> node_info_list_t;
    node_info_list_t node_info_list;
    app_ctxid_list_t game_ctxid_list;
    std::vector<game_ep_info> game_ep_list;

    BOOST_FOREACH(app_init_t const& app_init, game_list_)
    {
      game_ctxid_list.push_back(
        gce::svcid_t(app_init.node_id_, app_init.name_)
        );
      game_ep_info info;
      info.node_id_ = app_init.node_id_;
      info.ep_ = app_init.ep_;
      game_ep_list.push_back(info);
    }

    BOOST_FOREACH(router_init_t const& router_init, router_list_)
    {
      gce::svcid_t node_svc(router_init.first, gce::atom("node"));
      std::pair<node_info_list_t::iterator, bool> pr = 
        node_info_list.insert(std::make_pair(node_svc, node_info()));
      node_info& nod = pr.first->second;
      nod.router_list_.push_back(router_init);
    }

    BOOST_FOREACH(app_init_t const& app_init, game_list_)
    {
      gce::svcid_t node_svc(app_init.node_id_, gce::atom("node"));
      std::pair<node_info_list_t::iterator, bool> pr = 
        node_info_list.insert(std::make_pair(node_svc, node_info()));
      node_info& nod = pr.first->second;

      game_info info;
      info.game_list_ = game_ctxid_list;
      info.ep_ = app_init.ep_;
      info.svc_name_ = app_init.name_;
      nod.game_list_.push_back(info);
    }

    BOOST_FOREACH(app_init_t const& app_init, gate_list_)
    {
      gce::svcid_t node_svc(app_init.node_id_, gce::atom("node"));
      std::pair<node_info_list_t::iterator, bool> pr = 
        node_info_list.insert(std::make_pair(node_svc, node_info()));
      node_info& nod = pr.first->second;

      gate_info info;
      info.game_list_ = game_ctxid_list;
      info.ep_ = app_init.ep_;
      info.svc_name_ = app_init.name_;
      nod.gate_list_.push_back(info);
    }

    /// loop handle messages
    bool running = true;
    while (running)
    {
      gce::message msg;
      gce::aid_t sender = self.recv(msg);
      gce::match_t type = msg.get_type();
      if (type == gce::atom("logon"))
      {
        /// a node setup, ask what apps they can run
        gce::svcid_t node_svc;
        msg >> node_svc;
        node_info_list_t::iterator itr(node_info_list.find(node_svc));
        if (itr != node_info_list.end())
        {
          gce::reply(self, sender, gce::atom("ok"), std::string());
          gce::send(self, sender, gce::atom("app"), itr->second, game_ep_list);
        }
        else
        {
          gce::reply(self, sender, gce::atom("err"), std::string("node not found"));
        }
      }
      else if (type == gce::atom("stop"))
      {
        running = false;
        gce::reply(self, sender, gce::atom("ret"));
      }
      else
      {
        std::string errmsg("master unexpected message, type: ");
        errmsg += gce::atom(type);
        std::printf("%s\n", errmsg.c_str());
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("master except: %s\n", ex.what());
  }
  gce::deregister_service(self, gce::atom("master"));
}
///----------------------------------------------------------------------------
