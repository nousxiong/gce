///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/actor/all.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <cstdlib>
#include <iostream>

class client
{
public:
  client(
    std::string const& host,
    std::string const& port,
    std::size_t cln_num = 10,
    std::size_t echo_num = 3
    )
    : base_(gce::spawn(ctx_))
    , cln_num_(cln_num)
    , echo_num_(echo_num)
    , host_(host)
    , port_(port)
  {
    gce::aid_t base_id = base_.get_aid();
    for (std::size_t i=0; i<cln_num_; ++i)
    {
      gce::spawn(base_, boost::bind(&client::run, this, _1, base_id));
    }
  }

  ~client()
  {
    for (std::size_t i=0; i<cln_num_; ++i)
    {
      gce::recv(base_);
    }
  }

private:
  void run(gce::actor<gce::stackful>& self, gce::aid_t base_id)
  {
    try
    {
      /// wait 1 second for server setup
      gce::wait(self, gce::seconds_t(1));

      /// make a tcp socket to connnect to server
      gce::yield_t yield = self.get_yield();
      gce::io_service_t& ios = ctx_.get_io_service();

      boost::asio::ip::tcp::resolver reso(ios);
      boost::asio::ip::tcp::resolver::query query(host_, port_);
      boost::asio::ip::tcp::resolver::iterator itr = reso.async_resolve(query, yield);

      boost::asio::ip::tcp::socket sock(ios);
      boost::asio::async_connect(sock, itr, yield);

      char data[32];
      for (std::size_t i=0; i<echo_num_; ++i)
      {
        boost::asio::async_write(sock, boost::asio::buffer(data, 32), yield);
        boost::asio::async_read(sock, boost::asio::buffer(data, 32), yield);
        /// per second a echo
        gce::wait(self, gce::seconds_t(1));
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
    gce::send(self, base_id, gce::atom("done"));
  }

private:
  gce::context ctx_;
  gce::actor<gce::threaded> base_;
  std::size_t cln_num_;
  std::size_t echo_num_;
  std::string host_;
  std::string port_;
};

class server
{
  typedef boost::asio::ip::tcp::socket socket_t;
public:
  server(
    std::string const& host,
    boost::uint16_t port,
    std::size_t echo_num = 3
    )
    : acpr_(ctx_.get_io_service())
    , host_(host)
    , port_(port)
    , echo_num_(echo_num)
  {
    gce::actor<gce::threaded> base = gce::spawn(ctx_);
    gce::spawn(base, boost::bind(&server::run, this, _1));
  }

  ~server()
  {
    gce::errcode_t ignored_ec;
    acpr_.close(ignored_ec);
  }

private:
  void run(gce::actor<gce::stackful>& self)
  {
    try
    {
      gce::yield_t yield = self.get_yield();
      gce::io_service_t& ios = ctx_.get_io_service();

      boost::asio::ip::address addr;
      addr.from_string(host_);
      boost::asio::ip::tcp::endpoint ep(addr, port_);
      acpr_.open(ep.protocol());

      acpr_.set_option(boost::asio::socket_base::reuse_address(true));
      acpr_.bind(ep);

      acpr_.set_option(boost::asio::socket_base::receive_buffer_size(640000));
      acpr_.set_option(boost::asio::socket_base::send_buffer_size(640000));

      acpr_.listen(1024);

      acpr_.set_option(boost::asio::ip::tcp::no_delay(true));
      acpr_.set_option(boost::asio::socket_base::keep_alive(true));
      acpr_.set_option(boost::asio::socket_base::enable_connection_aborted(true));

      while (true)
      {
        gce::errcode_t ec;
        boost::shared_ptr<socket_t> sock(new socket_t(ios));
        acpr_.async_accept(*sock, yield[ec]);

        if (!ec)
        {
          std::cout << "new connection!\n";
          gce::spawn(self, boost::bind(&server::session, this, _1, sock));
        }
        else
        {
          break;
        }
      }
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

  void session(gce::actor<gce::stackful>& self, boost::shared_ptr<socket_t> sock)
  {
    try
    {
      gce::yield_t yield = self.get_yield();

      char data[32];
      for (std::size_t i=0; i<echo_num_; ++i)
      {
        boost::asio::async_read(*sock, boost::asio::buffer(data, 32), yield);
        boost::asio::async_write(*sock, boost::asio::buffer(data, 32), yield);
      }
      std::cout << "echo done!\n";
    }
    catch (std::exception& ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }

private:
  gce::context ctx_;
  boost::asio::ip::tcp::acceptor acpr_;
  std::string host_;
  boost::uint16_t port_;
  std::size_t echo_num_;
};

int main(int argc, char* argv[])
{
  try
  {
    char const* port = argc > 1 ? argv[1] : "10001";

    server svr("127.0.0.1", std::atoi(port));
    client cln("127.0.0.1", port);
  }
  catch (std::exception& ex)
  {
    std::cerr << ex.what() << std::endl;
  }
  return 0;
}
