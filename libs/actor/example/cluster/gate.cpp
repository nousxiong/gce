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
gate::gate(std::string cln_ep, app_ctxid_list_t game_list)
  : cln_ep_(cln_ep)
  , game_list_(game_list)
{
}
///----------------------------------------------------------------------------
gate::~gate()
{
}
///----------------------------------------------------------------------------
gce::aid_t gate::start(gce::self_t sire)
{
  gce::io_service_t& ios =
    sire.get_cache_pool()->get_context().get_io_service();
  acpr_.reset(new acceptor_t(ios));

  return
    gce::spawn(
      sire,
      boost::bind(
        &gate::run, this, _1
        ),
      gce::monitored
      );
}
//-----------------------------------------------------------------------------
void quit_callback(
  gce::self_t self, std::vector<gce::aid_t>& conn_group_list
  )
{
  std::vector<gce::response_t> res_list;
  BOOST_FOREACH(gce::aid_t aid, conn_group_list)
  {
    res_list.push_back(self.request(aid, gce::atom("stop")));
  }

  BOOST_FOREACH(gce::response_t res, res_list)
  {
    gce::message msg;
    self.recv(res, msg);
  }
  conn_group_list.clear();
}
///----------------------------------------------------------------------------
void gate::run(gce::self_t self)
{
  try
  {
    std::string host;
    boost::uint16_t port;

    /// parse protocol
    std::size_t pos = cln_ep_.find("://");
    if (pos == std::string::npos)
    {
      throw std::runtime_error("protocol error");
    }

    std::string protocol_name = cln_ep_.substr(0, pos);
    if (protocol_name == "tcp")
    {
      std::size_t begin = pos + 3;
      pos = cln_ep_.find(':', begin);
      if (pos == std::string::npos)
      {
        throw std::runtime_error("tcp address error");
      }

      std::string address = cln_ep_.substr(begin, pos - begin);

      begin = pos + 1;
      pos = cln_ep_.size();

      host = address;
      boost::uint16_t port =
        boost::lexical_cast<boost::uint16_t>(
          cln_ep_.substr(begin, pos - begin)
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
    acpr_->open(ep.protocol());

    acpr_->set_option(boost::asio::socket_base::reuse_address(true));
    acpr_->bind(ep);

    acpr_->listen(1024);

    acpr_->set_option(boost::asio::ip::tcp::no_delay(true));
    acpr_->set_option(boost::asio::socket_base::keep_alive(true));
    acpr_->set_option(boost::asio::socket_base::enable_connection_aborted(true));

    gce::errcode_t ignored_ec;
    gce::detail::scope acpr_scp(
      boost::bind(
        &acceptor_t::close,
        acpr_.get(), boost::ref(ignored_ec)
        )
      );

    /// spawn accept
    gce::spawn(
      self,
      boost::bind(&gate::accept, this, _1, conn_group_list),
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
        acpr_->close(ignored_ec);
        gce::recv(self, gce::exit);
        quit_callback(self, conn_group_list);
        gce::reply(self, sender, gce::atom("ret"));
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
    std::printf("gate::run except: %s\n", ex.what());
  }
}
///----------------------------------------------------------------------------
void gate::accept(gce::self_t self, std::vector<gce::aid_t> conn_group_list)
{
  try
  {
    std::size_t curr_group = 0;
    std::size_t const group_size = conn_group_list.size();
    boost::uint32_t sid_base = 0;
    gce::io_service_t& ios = acpr_->get_io_service();
    gce::yield_t yield = self.get_yield();

    while (true)
    {
      gce::errcode_t ec;
      if (++curr_group >= group_size)
      {
        curr_group = 0;
      }
      endpoint cid(curr_group, ++sid_base);

      socket_ptr skt(boost::make_shared<tcp_socket>(boost::ref(ios)));
      acpr_->async_accept(skt->get_socket(), yield[ec]);
      if (!ec)
      {
        std::printf(
          "new client conn, cid: <%u, %u>\n",
          cid.get_group_index(),
          cid.get_session_id()
          );
        gce::spawn(
          self,
          boost::bind(
            &conn::run, _1, skt,
            conn_group_list[curr_group],
            cid, game_list_
            )
          );
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
void gate::conn_group(gce::self_t self, gce::aid_t ga_id)
{
  typedef std::map<endpoint, gce::aid_t> conn_list_t;
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
        BOOST_FOREACH(conn_list_t::value_type& pr, conn_list)
        {
          self.send(pr.second, msg);
        }

        if (type == gce::atom("stop"))
        {
          gce::reply(self, sender, gce::atom("ret"));
        }
      }
      else if (type == gce::atom("add_conn"))
      {
        endpoint cid;
        msg >> cid;
        std::pair<conn_list_t::iterator, bool> pr =
          conn_list.insert(std::make_pair(cid, sender));
        BOOST_ASSERT(pr.second);

        std::printf(
          "add_conn: <%u, %u>\n",
          cid.get_group_index(),
          cid.get_session_id()
          );

        gce::reply(self, sender, gce::atom("ok"));
      }
      else if (type == gce::atom("rmv_conn"))
      {
        endpoint cid;
        msg >> cid;
        conn_list.erase(cid);

        std::printf(
          "rmv_conn: <%u, %u>\n",
          cid.get_group_index(),
          cid.get_session_id()
          );
      }
      else if (type == gce::atom("cln_msg"))
      {
        /// forward to conn
        boost::string_ref ignored;
        endpoint cid;
        msg >> ignored >> cid;

        conn_list_t::iterator itr(conn_list.find(cid));
        if (itr != conn_list.end())
        {
          self.send(itr->second, msg);
        }
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
