///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "conn.hpp"
#include <gce/detail/scope.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

///----------------------------------------------------------------------------
conn::conn(socket_ptr skt, gce::aid_t group_aid, app_ctxid_list_t game_list)
  : skt_(skt)
  , group_aid_(group_aid)
  , game_list_(game_list)
  , rcv_(skt_, game_list_)
{
}
///----------------------------------------------------------------------------
conn::~conn()
{
}
///----------------------------------------------------------------------------
gce::aid_t conn::spawn(
  gce::actor<gce::stackful>& sire, socket_ptr skt, 
  gce::aid_t group_aid, app_ctxid_list_t game_list
  )
{
  ptr_t c(boost::make_shared<conn>(skt, group_aid, game_list));
  return 
    gce::spawn<gce::stackless>(
      sire,
      boost::bind(&conn::run, c, _1)
      );
}
///----------------------------------------------------------------------------
void quit_callback(gce::actor<gce::stackless>& self, gce::aid_t group_aid)
{
  gce::send(self, group_aid, gce::atom("rmv_conn"));
}
///----------------------------------------------------------------------------
void conn::run(gce::actor<gce::stackless>& self)
{
  try
  {
    GCE_REENTER (self)
    {
      GCE_YIELD
      {
        gce::resp_t res =
          gce::request(
            self, group_aid_,
            gce::atom("add_conn")
            );

        self.recv(res, msg_);
      }

      if (msg_.get_type() != gce::atom("ok"))
      {
        throw std::runtime_error("add_conn error");
      }

      GCE_YIELD 
      {
        gce::spawn(
          self,
          boost::bind(&conn::timeout::run, &tmo_, _1),
          tmo_aid_,
          gce::linked
          );
      }

      GCE_YIELD
      {
        gce::spawn(
          self,
          boost::bind(
            &conn::recv::run, &rcv_, _1, 
            tmo_aid_, self.get_aid()
            ),
          tmp_aid_,
          gce::linked,
          true
          );
      }

      running_ = true;
      while (running_)
      {
        msg_ = gce::message();
        sender_ = gce::aid_t();
        GCE_YIELD self.recv(sender_, msg_);
        type_ = msg_.get_type();
        if (type_ == gce::exit || type_ == gce::atom("stop"))
        {
          running_ = false;
        }
        else if (type_ == gce::atom("fwd_msg"))
        {
          GCE_YIELD
          {
            gce::message m;
            msg_ >> m;
            ec_.clear();
            skt_->send(m, gce::adaptor(self, ec_));
          }

          if (ec_)
          {
            throw std::runtime_error("socket send error");
          }
        }
        else
        {
          std::string errmsg("conn::run unexpected message, type: ");
          errmsg += gce::atom(type_);
          throw std::runtime_error(errmsg);
        }
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("conn::run except: %s\n", ex.what());
    quit_callback(self, group_aid_);
  }
}
///----------------------------------------------------------------------------
void conn::timeout::run(gce::actor<gce::stackless>& self)
{
  try
  {
    GCE_REENTER (self)
    {
      curr_tmo_ = boost::chrono::seconds(60);
      max_count_ = 1;
      curr_count_ = 1;

      running_ = true;
      while (running_)
      {
        msg_ = gce::message();
        sender_ = gce::aid_t();
        GCE_YIELD self.recv(sender_, msg_, gce::pattern(curr_tmo_));
        gce::match_t type = msg_.get_type();
        if (sender_)
        {
          if (type == gce::exit)
          {
            running_ = false;
          }
          else if (type == gce::atom("reset"))
          {
            curr_count_ = max_count_;
          }
          else if (type == gce::atom("online"))
          {
            curr_tmo_ = boost::chrono::seconds(30);
            curr_count_ = 3;
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
          --curr_count_;
          if (curr_count_ == 0)
          {
            running_ = false;
          }
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
conn::recv::recv(socket_ptr skt, app_ctxid_list_t game_list)
  : skt_(skt)
  , game_list_(game_list)
{
}
///----------------------------------------------------------------------------
void conn::recv::run(
  gce::actor<gce::stackless>& self, 
  gce::aid_t tmo_aid, gce::aid_t conn_aid
  )
{
  try
  {
    GCE_REENTER (self)
    {
      tmo_aid_ = tmo_aid;
      conn_aid_ = conn_aid;
      stat_ = conned;

      while (true)
      {
        ec_.clear();
        msg_ = gce::message();
        GCE_YIELD skt_->recv(msg_, gce::adaptor(self, ec_));

        if (!ec_)
        {
          gce::send(self, tmo_aid_, gce::atom("reset"));
          type_ = msg_.get_type();
          if (type_ == gce::atom("cln_login"))
          {
            if (stat_ != conned)
            {
              throw std::runtime_error("conn status error, must be conned");
            }

            msg_ >> username_;
            game_svcid_ = select_game_app(game_list_, username_);

            msg_ << conn_aid_;
            GCE_YIELD
            {
              gce::resp_t res = self.request(game_svcid_, msg_);
              msg_ = gce::message();
              usr_aid_ = gce::aid_t();
              self.recv(res, usr_aid_, msg_, gce::seconds_t(5));
            }

            if (!usr_aid_)
            {
              throw std::runtime_error("client login failed");
            }
            else
            {
              std::string errmsg;
              msg_ >> errmsg;
              if (!errmsg.empty())
              {
                throw std::runtime_error(errmsg);
              }
            }

            stat_ = online;
            gce::message m(gce::atom("cln_login_ret"));
            m << std::string();
            gce::send(self, conn_aid_, gce::atom("fwd_msg"), m);
            gce::send(self, tmo_aid_, gce::atom("online"));
          }
          else if (type_ == gce::atom("chat"))
          {
            if (stat_ != online)
            {
              throw std::runtime_error("conn status error, must be online");
            }

            BOOST_FOREACH(gce::svcid_t svc, game_list_)
            {
              self.send(svc, msg_);
            }
          }
          else
          {
            if (stat_ != online)
            {
              throw std::runtime_error("conn status error, must be online");
            }

            /// forward to user
            self.send(usr_aid_, msg_);
          }
        }
        else
        {
          std::printf("conn::recv::run, socket err: %s\n", ec_.message().c_str());
          break;
        }
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("conn::recv::run except: %s\n", ex.what());
    std::string errstr = boost::diagnostic_information(ex);
    std::printf("kick, err: %s\n", errstr.c_str());

    gce::message m(gce::atom("kick"));
    m << errstr;
    gce::send(self, conn_aid_, gce::atom("fwd_msg"), m);
  }
}
///----------------------------------------------------------------------------
