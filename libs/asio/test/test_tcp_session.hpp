///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

namespace gce
{
namespace asio
{
class tcp_session_ut
{
typedef boost::asio::ip::tcp::socket tcp_socket_t;
typedef boost::asio::ip::tcp::resolver tcp_resolver_t;
public:
  static void run()
  {
    std::cout << "tcp_session_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "tcp_session_ut end." << std::endl;
  }

private:
  static void echo_client(stackful_actor self)
  {
    context& ctx = self.get_context();
    log::logger_t& lg = ctx.get_logger();
    io_service_t& ios = ctx.get_io_service();
    try
    {
      size_t ecount = 10;
      errcode_t ec;
      message msg;

      boost::shared_ptr<tcp_resolver_t::iterator> eitr;
      self->match("init").recv(eitr);

      session<parser::length, tcp_socket_t, stackful_actor> sn(
        self, 
        boost::make_shared<parser::simple>(), 
        boost::make_shared<tcp_socket_t>(boost::ref(ios)), 
        *eitr
        );

      for (size_t i=0; i<2; ++i)
      {
        sn.open();
        match_t type;
        self->match(sn_open, sn_close, type).raw(msg).recv();
        if (type == sn_close)
        {
          msg >> ec;
          boost::throw_exception(boost::system::system_error(ec));
        }

        std::string str("hello world!");
        message m;
        m << str;

        for (size_t i=0; i<ecount; ++i)
        {
          /// send msg
          sn.send(m);

          /// wait for echo
          self->match(sn_recv, sn_close, type).raw(msg).recv();
          if (type == sn_close)
          {
            msg >> ec;
            boost::throw_exception(boost::system::system_error(ec));
          }

          std::string echo_str;
          msg >> echo_str;
          GCE_VERIFY(echo_str == str);
        }

        m = message();
        m << "bye";
        sn.send(m);

        /// wait for server confirm or error
        self->match(sn_recv, sn_close, type).raw(msg).recv();
        if (type != sn_close)
        {
          sn.close();
          self->match(sn_close).recv();
        }
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "echo_client: " << ex.what();
    }
  }

  static void echo_session(stackful_actor self)
  {
    context& ctx = self.get_context();
    log::logger_t& lg = ctx.get_logger();
    try
    {
      errcode_t ec;
      message msg;
      match_t type;

      boost::shared_ptr<tcp_socket_t> tcp_skt;
      self->match("init").recv(tcp_skt);

      session<parser::length, tcp_socket_t, stackful_actor> sn(
        self, 
        boost::make_shared<parser::simple>(), 
        tcp_skt
        );

      sn.open();
      self->match(sn_open, sn_close, type).raw(msg).recv();
      if (type == sn_close)
      {
        msg >> ec;
        boost::throw_exception(boost::system::system_error(ec));
      }

      while (true)
      {
        self->match(sn_recv, sn_close, type).raw(msg).recv();
        if (type == sn_close)
        {
          msg >> ec;
          boost::throw_exception(boost::system::system_error(ec));
        }

        std::string str;
        msg >> str;
        if (str == "bye")
        {
          sn.send(msg);

          /// close session, true means grateful, 
          /// send all queued msgs before close socket
          sn.close(true);
          self->match(sn_close).recv();
          break;
        }

        sn.send(msg);
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "echo_session: " << ex.what();
    }
  }

  static void echo_server(stackful_actor self)
  {
    context& ctx = self.get_context();
    log::logger_t& lg = ctx.get_logger();
    try
    {
      aid_t sender = self->recv("init");

      size_t scount = 0;
      errcode_t ec;

      tcp::resolver rsv(self);
      tcp_resolver_t::query qry("0.0.0.0", "23333");
      rsv.async_resolve(qry);
      boost::shared_ptr<tcp_resolver_t::iterator> eitr;
      self->match(tcp::as_resolve).recv(ec, eitr);
      GCE_VERIFY(!ec).except(ec);

      tcp::acceptor acpr(self);
      boost::asio::ip::tcp::endpoint ep = **eitr;

      acpr->open(ep.protocol());

      acpr->set_option(boost::asio::socket_base::reuse_address(true));
      acpr->bind(ep);

      acpr->set_option(boost::asio::socket_base::receive_buffer_size(640000));
      acpr->set_option(boost::asio::socket_base::send_buffer_size(640000));

      acpr->listen(boost::asio::socket_base::max_connections);

      acpr->set_option(boost::asio::ip::tcp::no_delay(true));
      acpr->set_option(boost::asio::socket_base::keep_alive(true));
      acpr->set_option(boost::asio::socket_base::enable_connection_aborted(true));

      self->send(sender, "ready");

      while (true)
      {
        boost::shared_ptr<tcp_socket_t> skt = 
          boost::make_shared<tcp_socket_t>(boost::ref(ctx.get_io_service()));
        acpr.async_accept(*skt);

        match_t type;
        errcode_t ec;
        message msg;
        self->match(tcp::as_accept, "end", type).raw(msg).recv();
        if (type == atom("end"))
        {
          break;
        }

        msg >> ec;
        if (!ec)
        {
          aid_t cln = spawn(self, boost::bind(&tcp_session_ut::echo_session, _arg1), monitored);
          self->send(cln, "init", skt);
          ++scount;
        }
      }

      for (size_t i=0; i<scount; ++i)
      {
        self->recv(exit);
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "echo_server: " << ex.what();
    }
    GCE_INFO(lg) << "echo server end.";
  }

  static void test_base()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      size_t cln_count = 10;
      errcode_t ec;
      attributes attrs;
      attrs.lg_ = lg;
      context ctx_svr(attrs);
      context ctx_cln(attrs);

      threaded_actor base_svr = spawn(ctx_svr);
      threaded_actor base_cln = spawn(ctx_cln);

      aid_t svr = spawn(base_svr, boost::bind(&tcp_session_ut::echo_server, _arg1), monitored);
      base_svr->send(svr, "init");
      base_svr->recv("ready");

      tcp::resolver rsv(base_cln);
      boost::asio::ip::tcp::resolver::query qry("127.0.0.1", "23333");
      rsv.async_resolve(qry);
      boost::shared_ptr<tcp_resolver_t::iterator> eitr;
      base_cln->match(tcp::as_resolve).recv(ec, eitr);
      GCE_VERIFY(!ec).except(ec);

      for (size_t i=0; i<cln_count; ++i)
      {
        aid_t cln = spawn(base_cln, boost::bind(&tcp_session_ut::echo_client, _arg1), monitored);
        base_cln->send(cln, "init", eitr);
      }

      for (size_t i=0; i<cln_count; ++i)
      {
        base_cln->recv(exit);
      }

      base_svr->send(svr, "end");
      base_svr->recv(exit);
      GCE_INFO(lg) << "test_base end.";
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << "test_base: " << ex.what();
    }
  }
};
}
}
