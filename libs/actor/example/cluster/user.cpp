///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "user.hpp"

///----------------------------------------------------------------------------
void user::run(
  gce::actor<gce::stackful>& self, app_ctxid_list_t game_list,
  gce::aid_t old_usr_aid, gce::aid_t master,
  cid_t cid, std::string username, std::string passwd
  )
{
  try
  {
    if (old_usr_aid)
    {
      /// verify username and passwd, if valid then kick old session
      gce::resp_t res =
        gce::request(self, old_usr_aid, gce::atom("kick"));
      gce::recv(self);
    }
    else
    {
      /// check db/cache if username existed
      /// if existed, verify username and passwd; or register
    }

    /// verify or register ok, link cid;
    self.link(cid);

    /// response cln_login ok or error
    gce::aid_t sender = gce::recv(self, gce::atom("cln_login"));
    gce::reply(self, sender, gce::atom("ok"), std::string());

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
      else if (type == gce::atom("kick"))
      {
        running = false;
        gce::reply(self, sender, gce::atom("ok"));
      }
      else if (type == gce::atom("chat"))
      {
        gce::message m(gce::atom("fwd_msg"));
        m << msg;
        self.send(cid, m);
      }
      else if (type == gce::atom("chat_to"))
      {
        std::string target;
        msg >> target;
        if (target != username)
        {
          /// find game app and send chat msg
          gce::svcid_t game_svcid = select_game_app(game_list, target);
          self.send(game_svcid, msg);
        }
        else
        {
          /// send to self
          gce::message m(gce::atom("fwd_msg"));
          m << msg;
          self.send(cid, m);
        }
      }
      else if (type == gce::atom("cln_logout"))
      {
        running = false;
      }
      else
      {
        std::string errmsg("user::run unexpected message, type: ");
        errmsg += gce::atom(type);
        throw std::runtime_error(errmsg);
      }
    }

    std::printf("user quit\n");
  }
  catch (std::exception& ex)
  {
    std::printf("user::run except: %s\n", ex.what());
  }
  /// do some clean job before erase self from game_app
  gce::send(self, master, gce::atom("rmv_user"), username);
}
///----------------------------------------------------------------------------
