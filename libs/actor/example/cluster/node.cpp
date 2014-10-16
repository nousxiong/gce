///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "node.hpp"
#include "app.hpp"
#include "game.hpp"
#include "gate.hpp"
#include <boost/foreach.hpp>

///----------------------------------------------------------------------------
node::node(
  gce::attributes attrs, 
  std::string master_ep, 
  bool is_master, 
  gce::ctxid_t master_node_id, 
  std::vector<app_init_t> const& gate_list,
  std::vector<app_init_t> const& game_list,
  std::vector<router_init_t> const& router_list
  )
  : ctx_(attrs)
  , base_(gce::spawn(ctx_))
  , sig_(ctx_.get_io_service())
{
  sig_.add(SIGINT);
  sig_.add(SIGTERM);
#if defined(SIGQUIT)
  sig_.add(SIGQUIT);
#endif /// defined(SIGQUIT)

  if (is_master)
  {
    master_.reset(new master(gate_list, game_list, router_list));
    gce::bind(base_, master_ep);
  }
  else
  {
    gce::connect(base_, master_node_id, master_ep);
  }

  gce::svcid_t master(master_node_id, gce::atom("master"));
  gce::spawn(base_, boost::bind(&node::run, this, _1, master), gce::monitored);
}
///----------------------------------------------------------------------------
node::~node()
{
}
///----------------------------------------------------------------------------
void node::wait_end()
{
  gce::recv(base_);
}
///----------------------------------------------------------------------------
void node::run(gce::actor<gce::stackful>& self, gce::svcid_t master)
{
  typedef std::map<gce::match_t, gce::aid_t> app_list_t;
  app_list_t gate_list;
  app_list_t game_list;

  try
  {
    gce::register_service(self, gce::atom("node"));
    gce::svcid_t node_svc(ctx_.get_attributes().id_, gce::atom("node"));
    gce::aid_t master_aid;
    if (master_)
    {
      master_aid = master_->start(self);

      /// wait master setup
      gce::resp_t res = gce::request(self, master_aid, gce::atom("init"));
      gce::recv(self, res);
    }

    gce::aid_t sig =
      gce::spawn(
        self,
        boost::bind(
          &node::handle_signal, this, _1
          ),
        gce::monitored
        );

    /// tell master a new node online
    gce::resp_t res = gce::request(self, master, gce::atom("logon"), node_svc);
    std::string errmsg;
    gce::recv(self, res, errmsg, gce::seconds_t(5));
    if (!errmsg.empty())
    {
      throw std::runtime_error(errmsg);
    }

    std::printf("node %s setup\n", gce::atom(node_svc.ctxid_).c_str());

    bool running = true;
    while (running)
    {
      gce::message msg;
      gce::aid_t sender = self.recv(msg);
      gce::match_t type = msg.get_type();
      if (type == gce::exit)
      {
        if (sender == sig)
        {
          running = false;
        }
        else if (sender == master_aid)
        {
          gce::exit_code_t type;
          std::string errmsg;
          msg >> type >> errmsg;

          std::printf(
            "master quit: %s, %s, restart\n", 
            gce::atom(type).c_str(), 
            errmsg.c_str()
            );

          master_aid = master_->start(self);
        }
        else
        {
          gce::exit_code_t type;
          std::string errmsg;
          msg >> type >> errmsg;

          /// one of apps quit;
          /// here, just quit;
          /// you can restart the error app
          std::printf(
            "app quit: %s, %s\n", 
            gce::atom(type).c_str(), 
            errmsg.c_str()
            );
        }
      }
      else if (type == gce::atom("app"))
      {
        /// master want to create some apps; 
        /// using unique name;
        /// first of all, rmv old ones; note: apps are bind with their nodes,
        /// one app always create/stop on one node
        node_info ni;
        std::vector<game_ep_info> game_ep_list;
        msg >> ni >> game_ep_list;

        /// make routers
        BOOST_FOREACH(router_init_t& init, ni.router_list_)
        {
          gce::bind(self, init.second, true);
        }

        /// stop all given apps
        std::vector<gce::resp_t> response_list;
        BOOST_FOREACH(gate_info& info, ni.gate_list_)
        {
          app_list_t::iterator itr(gate_list.find(info.svc_name_));
          if (itr != gate_list.end())
          {
            response_list.push_back(
              gce::request(self, itr->second, gce::atom("stop"))
              );
          }
        }

        BOOST_FOREACH(game_info& info, ni.game_list_)
        {
          app_list_t::iterator itr(game_list.find(info.svc_name_));
          if (itr != game_list.end())
          {
            response_list.push_back(
              gce::request(self, itr->second, gce::atom("stop"))
              );
          }
        }

        BOOST_FOREACH(gce::resp_t res, response_list)
        {
          gce::message msg;
          self.recv(res, msg);
        }

        /// start given game apps
        if (!ni.game_list_.empty())
        {
          /// connect to game router
          BOOST_FOREACH(router_init_t& init, ni.router_list_)
          {
            gce::connect(self, init.first, init.second, true);
          }
        }

        BOOST_FOREACH(game_info& info, ni.game_list_)
        {
          gce::bind(self, info.ep_);
          gce::aid_t aid = game::start(self, info.svc_name_, info.game_list_);
          std::pair<app_list_t::iterator, bool> pr = 
            game_list.insert(std::make_pair(info.svc_name_, aid));
          pr.first->second = aid;
        }

        /// start given gate apps
        if (!ni.gate_list_.empty())
        {
          /// connect to games
          BOOST_FOREACH(game_ep_info& info, game_ep_list)
          {
            gce::connect(self, info.node_id_, info.ep_);
          }
        }

        BOOST_FOREACH(gate_info& info, ni.gate_list_)
        {
          gce::aid_t aid = gate::start(self, info.svc_name_, info.ep_, info.game_list_);
          std::pair<app_list_t::iterator, bool> pr = 
            gate_list.insert(std::make_pair(info.svc_name_, aid));
          pr.first->second = aid;
        }
      }
      else
      {
        std::string errmsg("node unexpected message, type: ");
        errmsg += gce::atom(type);
        std::printf("%s\n", errmsg.c_str());
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("node except: %s\n", ex.what());
  }

  std::printf("node %s stopping...\n", gce::atom(ctx_.get_attributes().id_).c_str());

  std::vector<gce::resp_t> response_list;
  BOOST_REVERSE_FOREACH(app_list_t::value_type& pr, gate_list)
  {
    response_list.push_back(
      gce::request(self, pr.second, gce::atom("stop"))
      );
  }

  BOOST_REVERSE_FOREACH(app_list_t::value_type& pr, game_list)
  {
    response_list.push_back(
      gce::request(self, pr.second, gce::atom("stop"))
      );
  }

  if (master_)
  {
    response_list.push_back(
      gce::request(self, master, gce::atom("stop"))
      );
  }


  BOOST_FOREACH(gce::resp_t res, response_list)
  {
    gce::message msg;
    self.recv(res, msg);
  }
}
///----------------------------------------------------------------------------
void node::handle_signal(gce::actor<gce::stackful>& self)
{
  gce::yield_t yield = self.get_yield();
  gce::errcode_t ec;
  sig_.async_wait(yield[ec]);
}
///----------------------------------------------------------------------------
