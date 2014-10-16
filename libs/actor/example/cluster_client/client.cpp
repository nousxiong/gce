///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "client.hpp"
#include <gce/detail/scope.hpp>
#include <boost/asio.hpp>
#include <sstream>
#include <iostream>

///----------------------------------------------------------------------------
client::client(std::string gate_ep)
  : gate_ep_(parse_potocol(gate_ep))
  , ctx_(get_attrs())
  , base_(gce::spawn(ctx_))
{
}
///----------------------------------------------------------------------------
client::~client()
{
}
///----------------------------------------------------------------------------
void client::run()
{
  gce::aid_t cln_aid =
    gce::spawn(
      base_,
      boost::bind(&client::pri_run, this, _1),
      gce::monitored
      );

  std::cout << "Usage commands: login, chat, chat_to, logout\n" <<
    " login <username> <passwd>\n" <<
    " chat <chat content>\n" <<
    " chat_to <username> <chat content>\n" <<
    " logout" << std::endl;


  try
  {
    std::string cmd;
    do
    {
      std::getline(std::cin, cmd);
      if (!std::cin.good() || cmd == "quit")
      {
        break;
      }
      command(cmd, cln_aid);
    }
    while (true);
  }
  catch (std::exception& ex)
  {
    std::printf("client::run except: %s\n", ex.what());
  }

  gce::send(base_, cln_aid, gce::atom("stop"));
  gce::recv(base_);
}
///----------------------------------------------------------------------------
void client::command(std::string cmd, gce::aid_t cln_aid)
{
  std::stringstream cmd_str(cmd);
  std::string tok;
  std::vector<std::string> tok_list;

  while (std::getline(cmd_str, tok, ' '))
  {
    tok_list.push_back(tok);
  }

  if (!tok_list.empty())
  {
    if (tok_list[0] == "login")
    {
      if (tok_list.size() < 3)
      {
        throw std::runtime_error("not enough params; Usage: login <username> <passwd>");
      }
      username_ = tok_list[1];
      gce::send(base_, cln_aid, gce::atom("cln_login"), tok_list[1], tok_list[2]);
    }
    else if (tok_list[0] == "chat")
    {
      if (tok_list.size() < 2)
      {
        throw std::runtime_error("not enough params; Usage: chat <content>");
      }
      gce::send(base_, cln_aid, gce::atom("chat"), username_, tok_list[1]);
    }
    else if (tok_list[0] == "chat_to")
    {
      if (tok_list.size() < 3)
      {
        throw std::runtime_error("not enough params; Usage: chat_to <username> <content>");
      }
      gce::send(base_, cln_aid, gce::atom("chat_to"), tok_list[1], username_, tok_list[2]);
    }
    else if (tok_list[0] == "logout")
    {
      gce::send(base_, cln_aid, gce::atom("cln_logout"), tok_list[1]);
    }
  }
}
///----------------------------------------------------------------------------
boost::array<std::string, 2> client::parse_potocol(std::string gate_ep)
{
  boost::array<std::string, 2> ret;

  /// parse protocol
  std::size_t pos = gate_ep.find("://");
  if (pos == std::string::npos)
  {
    throw std::runtime_error("protocol error");
  }

  std::string protocol_name = gate_ep.substr(0, pos);
  if (protocol_name == "tcp")
  {
    std::size_t begin = pos + 3;
    pos = gate_ep.find(':', begin);
    if (pos == std::string::npos)
    {
      throw std::runtime_error("tcp address error");
    }

    std::string address = gate_ep.substr(begin, pos - begin);

    begin = pos + 1;
    pos = gate_ep.size();

    ret[0] = address;
    ret[1] = gate_ep.substr(begin, pos - begin);
    return ret;
  }
  else
  {
    throw std::runtime_error("unknown protocol");
  }
}
///----------------------------------------------------------------------------
void quit_callback(tcp_socket& skt)
{
  skt.close();
}
///----------------------------------------------------------------------------
void client::pri_run(gce::actor<gce::stackful>& self)
{
  try
  {
    gce::yield_t yield = self.get_yield();
    gce::io_service_t& ios = ctx_.get_io_service();
    tcp_socket skt(ios);

    boost::asio::ip::tcp::resolver reso(ios);
    boost::asio::ip::tcp::resolver::query query(gate_ep_[0], gate_ep_[1]);
    boost::asio::ip::tcp::resolver::iterator itr = reso.async_resolve(query, yield);
    boost::asio::async_connect(skt.get_socket(), itr, yield);

    gce::detail::scope scp(boost::bind(&quit_callback, boost::ref(skt)));
    gce::spawn(
      self,
      boost::bind(
        &client::recv, this, _1, boost::ref(skt)
        ),
      gce::monitored,
      true
      );

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
      }
      else if (
        type == gce::atom("cln_login") ||
        type == gce::atom("chat") ||
        type == gce::atom("chat_to")
        )
      {
        skt.send(msg, yield);
      }
      else
      {
        std::string errmsg("client::pri_run unexpected message, type: ");
        errmsg += gce::atom(type);
        throw std::runtime_error(errmsg);
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("client::pri_run except: %s\n", ex.what());
  }
}
///----------------------------------------------------------------------------
void client::recv(gce::actor<gce::stackful>& self, tcp_socket& skt)
{
  try
  {
    gce::yield_t yield = self.get_yield();

    while (true)
    {
      gce::errcode_t ec;
      gce::message msg;
      msg = skt.recv(yield[ec]);

      if (!ec)
      {
        gce::match_t type = msg.get_type();
        if (type == gce::atom("kick"))
        {
          std::string errmsg;
          msg >> errmsg;
          std::printf("server kick! errmsg: %s\n", errmsg.c_str());
          break;
        }
        else if (type == gce::atom("cln_login_ret"))
        {
          std::string errmsg;
          msg >> errmsg;
          if (!errmsg.empty())
          {
            std::printf("login error! errmsg: %s\n", errmsg.c_str());
            break;
          }
          else
          {
            std::printf("login ok!\n");
          }
        }
        else if (type == gce::atom("chat"))
        {
          std::string username, chat_msg;
          msg >> username >> chat_msg;
          std::printf("[%s] %s\n", username.c_str(), chat_msg.c_str());
        }
        else if (type == gce::atom("chat_to"))
        {
          std::string self_name, username, chat_msg;
          msg >> self_name >> username >> chat_msg;
          std::printf("[%s]<in private> %s\n", username.c_str(), chat_msg.c_str());
        }
        else
        {
          std::string errmsg("client::recv unexpected message, type: ");
          errmsg += gce::atom(type);
          throw std::runtime_error(errmsg);
        }
      }
      else
      {
        std::printf("client::recv, socket err: %s\n", ec.message().c_str());
        break;
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("client::recv except: %s\n", ex.what());
  }
}
///----------------------------------------------------------------------------
gce::attributes client::get_attrs()
{
  gce::attributes attrs;
  attrs.thread_num_ = 1;
  return attrs;
}
///----------------------------------------------------------------------------
