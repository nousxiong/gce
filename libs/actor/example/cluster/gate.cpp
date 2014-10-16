///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include "gate.hpp"
#include <gce/detail/scope.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/utility/string_ref.hpp>
#include <vector>

///----------------------------------------------------------------------------
gce::aid_t gate::start(
  gce::actor<gce::stackful>& sire, gce::match_t svc_name, 
  std::string cln_ep, app_ctxid_list_t game_list
  )
{
  return
    gce::spawn(
      sire,
      boost::bind(
        &gate::run, _1, svc_name, 
        cln_ep, game_list
        ),
      gce::monitored
      );
}
///----------------------------------------------------------------------------
void quit_callback(
  gce::actor<gce::stackful>& self, std::vector<gce::aid_t>& conn_group_list
  )
{
  std::vector<gce::resp_t> res_list;
  BOOST_FOREACH(gce::aid_t aid, conn_group_list)
  {
    res_list.push_back(self.request(aid, gce::atom("stop")));
  }

  BOOST_FOREACH(gce::resp_t res, res_list)
  {
    gce::message msg;
    self.recv(res, msg);
  }
  conn_group_list.clear();
}
///----------------------------------------------------------------------------
void gate::run(gce::actor<gce::stackful>& self, gce::match_t svc_name, std::string cln_ep, app_ctxid_list_t game_list)
{
  try
  {
    gce::io_service_t& ios = self.get_context()->get_io_service();
    acceptor_t acpr(ios);

    std::string host;
    boost::uint16_t port;

    /// parse protocol
    std::size_t pos = cln_ep.find("://");
    if (pos == std::string::npos)
    {
      throw std::runtime_error("protocol error");
    }

    std::string protocol_name = cln_ep.substr(0, pos);
    if (protocol_name == "tcp")
    {
      std::size_t begin = pos + 3;
      pos = cln_ep.find(':', begin);
      if (pos == std::string::npos)
      {
        throw std::runtime_error("tcp address error");
      }

      std::string address = cln_ep.substr(begin, pos - begin);

      begin = pos + 1;
      pos = cln_ep.size();

      host = address;
      port =
        boost::lexical_cast<boost::uint16_t>(
          cln_ep.substr(begin, pos - begin)
          );
    }
    else
    {
      throw std::runtime_error("unknown protocol");
    }

    /// spawn conn_group
    std::vector<gce::aid_t> conn_group_list;
    gce::detail::scope conn_scp(
      boost::bind(
        &quit_callback, boost::ref(self),
        boost::ref(conn_group_list)
        )
      );
    std::size_t conn_group_size = boost::thread::hardware_concurrency();
    for (std::size_t i=0; i<conn_group_size; ++i)
    {
      conn_group_list.push_back(
        gce::spawn(
          self,
          boost::bind(
            &gate::conn_group, _1, self.get_aid()
            ),
          gce::linked
          )
        );
    }

    /// bind and listen tcp address
    boost::asio::ip::address addr;
    addr.from_string(host);
    boost::asio::ip::tcp::endpoint ep(addr, port);
    acpr.open(ep.protocol());

    acpr.set_option(boost::asio::socket_base::reuse_address(true));
    acpr.bind(ep);

    acpr.listen(1024);

    acpr.set_option(boost::asio::ip::tcp::no_delay(true));
    acpr.set_option(boost::asio::socket_base::keep_alive(true));
    acpr.set_option(boost::asio::socket_base::enable_connection_aborted(true));

    gce::errcode_t ignored_ec;
    gce::detail::scope acpr_scp(
      boost::bind(
        &acceptor_t::close,
        &acpr, boost::ref(ignored_ec)
        )
      );

    /// spawn accept
    gce::spawn(
      self,
      boost::bind(
        &gate::accept, _1, 
        boost::ref(acpr), game_list, conn_group_list
        ),
      gce::monitored,
      true
      );

    std::printf(
      "gate %s setup, listen client conn on: %s\n", 
      gce::atom(svc_name).c_str(), cln_ep.c_str()
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
        acpr.close(ignored_ec);
        gce::recv(self, gce::exit);
        quit_callback(self, conn_group_list);
        gce::reply(self, sender, gce::atom("ret"));
      }
      else
      {
        std::string errmsg("gate unexpected message, type: ");
        errmsg += gce::atom(type);
        std::printf("%s\n", errmsg.c_str());
      }
    }

    std::printf("gate %s quit\n", gce::atom(svc_name).c_str());
  }
  catch (std::exception& ex)
  {
    std::printf("gate except: %s\n", ex.what());
  }
}
///----------------------------------------------------------------------------
void gate::accept(
  gce::actor<gce::stackful>& self, 
  acceptor_t& acpr, 
  app_ctxid_list_t game_list,
  std::vector<gce::aid_t> conn_group_list
  )
{
  try
  {
    gce::ctxid_t ctxid = self.get_context()->get_attributes().id_;
    std::size_t curr_group = 0;
    std::size_t const group_size = conn_group_list.size();
    boost::uint32_t sid_base = 0;
    gce::io_service_t& ios = acpr.get_io_service();
    gce::yield_t yield = self.get_yield();

    while (true)
    {
      gce::errcode_t ec;
      if (++curr_group >= group_size)
      {
        curr_group = 0;
      }

      socket_ptr skt(boost::make_shared<tcp_socket>(boost::ref(ios)));
      acpr.async_accept(skt->get_socket(), yield[ec]);
      if (!ec)
      {
        std::printf("new client conn\n");
        conn::spawn(self, skt, conn_group_list[curr_group], game_list);
      }
      else
      {
        std::printf("client accept error: %s\n", ec.message().c_str());
        break;
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("gate::accept except: %s\n", ex.what());
  }
}
///----------------------------------------------------------------------------
void gate::conn_group(gce::actor<gce::stackful>& self, gce::aid_t ga_id)
{
  typedef std::set<gce::aid_t> conn_list_t;
  try
  {
    conn_list_t conn_list;
    bool running = true;
    while (running)
    {
      gce::message msg;
      gce::aid_t sender = self.recv(msg);
      gce::match_t type = msg.get_type();
      if (type == gce::exit || type == gce::atom("stop"))
      {
        running = false;
        BOOST_FOREACH(gce::aid_t cid, conn_list)
        {
          self.send(cid, msg);
        }

        if (type == gce::atom("stop"))
        {
          gce::reply(self, sender, gce::atom("ret"));
        }
      }
      else if (type == gce::atom("add_conn"))
      {
        std::pair<conn_list_t::iterator, bool> pr =
          conn_list.insert(sender);
        BOOST_ASSERT(pr.second);

        gce::reply(self, sender, gce::atom("ok"));
      }
      else if (type == gce::atom("rmv_conn"))
      {
        conn_list.erase(sender);
      }
      else
      {
        std::string errmsg("gate::run unexpected message, type: ");
        errmsg += gce::atom(type);
        throw std::runtime_error(errmsg);
      }
    }
  }
  catch (std::exception& ex)
  {
    std::printf("gate::conn_group except: %s\n", ex.what());
  }
}
///----------------------------------------------------------------------------
