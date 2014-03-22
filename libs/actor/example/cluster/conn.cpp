///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "conn.hpp"
#include "hash.hpp"
#include <gce/detail/scope.hpp>

///----------------------------------------------------------------------------
conn::conn()
{
}
///----------------------------------------------------------------------------
conn::~conn()
{
}
//-----------------------------------------------------------------------------
void quit_callback(gce::self_t self, gce::aid_t group_aid, endpoint cid)
{
  gce::send(self, group_aid, gce::atom("rmv_conn"), cid);
}
///----------------------------------------------------------------------------
void conn::run(
  gce::self_t self, socket_ptr skt, gce::aid_t group_aid,
  endpoint cid, app_ctxid_list_t game_list
  )
{
  try
  {
    gce::yield_t yield = self.get_yield();

    gce::detail::scope scp(boost::bind(&tcp_socket::close, skt));
    gce::response_t res =
      gce::request(
        self, group_aid,
        gce::atom("add_conn"), cid
        );
    gce::message msg;
    self.recv(res, msg);
    if (msg.get_type() != gce::atom("ok"))
    {
      throw std::runtime_error("add_conn error");
    }

    gce::detail::scope quit_scp(
      boost::bind(
        &quit_callback, boost::ref(self),
        group_aid, cid
        )
      );

    gce::aid_t tmo_aid =
      gce::spawn(
        self,
        boost::bind(&conn::timeout, _1),
        gce::linked
        );

    gce::aid_t recv_aid =
      gce::spawn(
        self,
        boost::bind(
          &conn::recv, _1, skt, cid,
          tmo_aid, self.get_aid(), game_list
          ),
        gce::linked,
        true
        );

    bool running = true;
    while (running)
    {
      gce::message msg;
      gce::aid_t sender = self.recv(msg);
      gce::match_t type = msg.get_type();
      if (type == gce::exit || type == gce::atom("stop"))
      {
        running = false;
      }
      else if (type == gce::atom("fwd_msg"))
      {
        gce::message m;
        msg >> m;
        skt->send(m, yield);
      }
      else
      {
        std::string errmsg("conn::run unexpected message, type: ");
        errmsg += gce::atom(type);
        throw std::runtime_error(errmsg);
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("conn::run except: %s\n", ex.what());
  }
}
///----------------------------------------------------------------------------
void conn::timeout(gce::self_t self)
{
  try
  {
    boost::chrono::seconds curr_tmo(30);
    std::size_t max_count = 1;
    std::size_t curr_count = 1;

    bool running = true;
    while (running)
    {
      gce::message msg;
      gce::aid_t sender = self.recv(msg, gce::match(curr_tmo));
      gce::match_t type = msg.get_type();
      if (sender)
      {
        if (type == gce::exit)
        {
          running = false;
        }
        else if (type == gce::atom("reset"))
        {
          curr_count = max_count;
        }
        else if (type == gce::atom("logon"))
        {
          curr_tmo = boost::chrono::seconds(30);
          curr_count = 3;
        }
        else
        {
          std::string errmsg("conn::timeout unexpected message, type: ");
          errmsg += gce::atom(type);
          throw std::runtime_error(errmsg);
        }
      }
      else
      {
        --curr_count;
        if (curr_count == 0)
        {
          running = false;
        }
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("conn::timeout except: %s\n", ex.what());
  }
}
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
void conn::recv(
  gce::self_t self, socket_ptr skt, endpoint cid,
  gce::aid_t tmo_aid, gce::aid_t conn_aid,
  app_ctxid_list_t game_list
  )
{
  try
  {
    gce::yield_t yield = self.get_yield();
    gce::svcid_t game_svcid;
    status stat = conned;

    while (true)
    {
      gce::errcode_t ec;
      gce::message msg;
      msg = skt->recv(yield[ec]);

      if (!ec)
      {
        gce::send(self, tmo_aid, gce::atom("reset"));
        gce::match_t type = msg.get_type();
        if (type == gce::atom("cln_login"))
        {
          if (stat != conned)
          {
            throw std::runtime_error("conn status error, must be conned");
          }

          std::string username;
          msg >> username;
          game_svcid = select_game_app(game_list, username);

          msg << conn_aid << cid;
          gce::response_t res = self.request(game_svcid, msg);
          std::string errmsg;
          gce::recv(self, res, errmsg, gce::seconds_t(60));
          if (!errmsg.empty())
          {
            throw std::runtime_error(errmsg);
          }
          stat = online;
          gce::send(self, tmo_aid, gce::atom("logon"));
        }
        else
        {
          if (stat != online)
          {
            throw std::runtime_error("conn status error, must be online");
          }
          /// forward to game_app
          self.send(game_svcid, msg);
        }
      }
      else
      {
        std::printf("conn::recv, socket err: %s\n", ec.message().c_str());
        break;
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("conn::recv except: %s\n", ex.what());
    std::string errstr = boost::diagnostic_information(ex);

    std::printf(
      "kick, cid: <%u, %u>, err: %s\n",
      cid.get_group_index(),
      cid.get_session_id(),
      errstr.c_str()
      );

    gce::message m(gce::atom("kick"));
    m << errstr;
    gce::send(self, conn_aid, gce::atom("fwd_msg"), m);
  }
}
///----------------------------------------------------------------------------
