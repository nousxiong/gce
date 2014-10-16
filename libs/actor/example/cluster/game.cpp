///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "game.hpp"
#include "user.hpp"
#include <boost/foreach.hpp>
#include <map>

///----------------------------------------------------------------------------
gce::aid_t game::start(gce::actor<gce::stackful>& sire, gce::match_t svc_name, app_ctxid_list_t game_list)
{
  return
    gce::spawn(
      sire,
      boost::bind(
        &game::run, _1, svc_name, game_list
        ),
      gce::monitored
      );
}
///----------------------------------------------------------------------------
void game::run(gce::actor<gce::stackful>& self, gce::match_t svc_name, app_ctxid_list_t game_list)
{
  try
  {
    typedef std::map<std::string, gce::aid_t> user_list_t;
    user_list_t user_list;

    gce::register_service(self, svc_name);

    std::printf("game %s setup\n", gce::atom(svc_name).c_str());

    /// loop handle messages
    bool running = true;
    while (running)
    {
      gce::message msg;
      gce::aid_t sender = self.recv(msg);
      gce::match_t type = msg.get_type();
      if (type == gce::exit)
      {
        running = false;
      }
      else if (type == gce::atom("stop"))
      {
        running = false;
        gce::reply(self, sender, gce::atom("ret"));
      }
      else if (type == gce::atom("cln_login"))
      {
        std::string username, passwd;
        cid_t cid;
        msg >> username >> passwd >> cid;

        std::printf("new client login, username: %s\n", username.c_str());

        gce::aid_t old_usr_aid;
        std::pair<user_list_t::iterator, bool> pr =
          user_list.insert(std::make_pair(username, gce::aid_t()));
        if (!pr.second)
        {
          old_usr_aid = pr.first->second;
        }

        pr.first->second =
          gce::spawn(
            self,
            boost::bind(
              &user::run, _1, game_list, old_usr_aid,
              self.get_aid(), cid, username, passwd
              )
            );
        self.relay(pr.first->second, msg);
      }
      else if (type == gce::atom("chat"))
      {
        /// broadcast chat msg
        BOOST_FOREACH(user_list_t::value_type& pr, user_list)
        {
          self.send(pr.second, msg);
        }
      }
      else if (type == gce::atom("chat_to"))
      {
        /// find target and send chat msg
        std::string username;
        msg >> username;
        user_list_t::iterator itr(user_list.find(username));
        if (itr != user_list.end())
        {
          self.send(itr->second, msg);
        }
      }
      else if (type == gce::atom("rmv_user"))
      {
        std::string username;
        msg >> username;
        user_list_t::iterator itr(user_list.find(username));
        if (itr != user_list.end() && itr->second == sender)
        {
          user_list.erase(itr);
        }
      }
      else
      {
        std::string errmsg("game unexpected message, type: ");
        errmsg += gce::atom(type);
        std::printf("%s\n", errmsg.c_str());
      }
    }

    std::printf("game %s quit\n", gce::atom(svc_name).c_str());
  }
  catch (std::exception& ex)
  {
    std::printf("game except: %s\n", ex.what());
  }
  gce::deregister_service(self, svc_name);
}
///----------------------------------------------------------------------------
