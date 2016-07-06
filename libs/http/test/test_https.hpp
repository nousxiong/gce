///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#include <gce/amsg/all.hpp>
#include <boost/make_shared.hpp>

namespace gce
{
namespace http
{
const char * https_get_raw = "GET /favicon.ico HTTP/1.1\r\n"
         "Host: 0.0.0.0=5000\r\n"
         "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
         "Accept-Language: en-us,en;q=0.5\r\n"
         "Accept-Encoding: gzip,deflate\r\n"
         "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
         "Keep-Alive: 300\r\n"
         "Connection: keep-alive\r\n"
         "\r\n";

const char * https_post_raw =  "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
         "Accept: */*\r\n"
         "Transfer-Encoding: identity\r\n"
         "Content-Length: 6\r\n"
         "\r\n"
         "World\n";

const char * https_trunk_head =  "POST /chunked_w_trailing_headers HTTP/1.1\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5\r\nhello\r\n"
         "7\r\n world\n\r\n"
         "0\r\n"
         "Vary: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n";

const char * https_post_bye =  "POST / HTTP/1.1\r\n"
         "Accept: */*\r\n"
         "Transfer-Encoding: identity\r\n"
         "Content-Length: 4\r\n"
         "\r\n"
         "bye\n";

const char * https_wrong_trunk_head =  "POSD /chunked_w_trailing_headers HTTP/1.2\r\n"
         "Transfer-Encoding: chunked\r\n"
         "\r\n"
         "5\r\nhello\r\n"
         "6\r\n world\r\n"
         "0\r\n"
         "Vary: *\r\n"
         "Content-Type: text/plain\r\n"
         "\r\n";

class https_ut
{
typedef boost::asio::ip::tcp::socket tcp_socket_t;
typedef boost::asio::ssl::stream<tcp_socket_t> ssl_socket_t;
typedef boost::asio::ip::tcp::resolver tcp_resolver_t;
public:
  static void run()
  {
    std::cout << "https_ut begin." << std::endl;
    for (std::size_t i=0; i<test_count; ++i)
    {
      test_base();
      if (test_count > 1) std::cout << "\r" << i;
    }
    if (test_count > 1) std::cout << std::endl;
    std::cout << "https_ut end." << std::endl;
  }

private:
  static bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx, log::logger_t& lg)
  {
    // In this example we will simply print the certificate's subject name.
    /*char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);*/
    //GCE_INFO(lg) << "Verifying " << subject_name << ", " << preverified;
    //return preverified;
    return true;
  }

  static void echo_session(stackful_actor self)
  {
    log::logger_t& lg = self.get_context().get_logger();
    try
    {
      boost::shared_ptr<ssl_socket_t> ssl_skt;
      self->match("init").recv(ssl_skt);

      asio::ssl::stream<> skt(self, ssl_skt);

      errcode_t ec;
      skt.async_handshake(boost::asio::ssl::stream_base::server);
      self->match(asio::ssl::as_handshake).recv(ec);
      GCE_VERIFY(!ec).except(ec);

      server::connection conn(self, ssl_skt);
      request_ptr req;

      while (true)
      {
        errcode_t ec;
        conn.recv();
        self->match(as_request).recv(ec, req);
        GCE_VERIFY(!ec).except(ec);
        if (req->upgrade_)
        {
          /// If upgrade e.g. websocket, user must send handshake(websocket) here.
          GCE_VERIFY(false);
        }

        if (!req->errmsg_.empty())
        {
          GCE_ERROR(lg) << "Http server " << req->errmsg_;
          conn.close();
          self->recv(as_close);
          break;
        }

        /*std::string const& method = req->method_;
        int major = req->http_major_;
        int minor = req->http_minor_;
        std::string const& uri = req->uri_;
        std::vector<header> const& headers = req->headers_;
        BOOST_FOREACH(header const& hdr, headers)
        {
          std::cout << "  " << hdr.name_ << ": " << hdr.value_ << std::endl;
        }*/

        reply_ptr rep = conn.make_reply(reply::ok);
        write(rep, req->body_);
        boost::string_ref cont = rep->get_content();
        
        rep->add_header("Content-Length", boost::lexical_cast<std::string>(cont.size()));
        rep->add_header("Content-Type", mime::ext2type("html"));

        conn.send(rep);

        if (cont == "bye\n")
        {
          conn.close();
          self->recv(as_close);
          break;
        }
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << ex.what();
    }
  }

  static std::string get_password()
  {
    return "test";
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

      asio::tcp::resolver rsv(self);
      tcp_resolver_t::query qry("0.0.0.0", "23333");
      rsv.async_resolve(qry);
      boost::shared_ptr<tcp_resolver_t::iterator> eitr;
      self->match(asio::tcp::as_resolve).recv(ec, eitr);
      GCE_VERIFY(!ec).except(ec);

      asio::tcp::acceptor acpr(self);
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

      /// ssl context
      boost::asio::ssl::context ssl_ctx(boost::asio::ssl::context::sslv23);
      ssl_ctx.set_options(
          boost::asio::ssl::context::default_workarounds
          | boost::asio::ssl::context::no_sslv2
          | boost::asio::ssl::context::single_dh_use);
      ssl_ctx.set_password_callback(boost::bind(&https_ut::get_password));
      ssl_ctx.use_certificate_chain_file("test_https/server.pem");
      ssl_ctx.use_private_key_file("test_https/server.pem", boost::asio::ssl::context::pem);
      ssl_ctx.use_tmp_dh_file("test_https/dh512.pem");

      self->send(sender, "ready");

      while (true)
      {
        boost::shared_ptr<ssl_socket_t> skt = 
          boost::make_shared<ssl_socket_t>(boost::ref(ctx.get_io_service()), boost::ref(ssl_ctx));
        acpr.async_accept(skt->lowest_layer());

        match_t type;
        errcode_t ec;
        message msg;
        self->match(asio::tcp::as_accept, "end", type).raw(msg).recv();
        if (type == atom("end"))
        {
          break;
        }

        msg >> ec;
        if (!ec)
        {
          aid_t cln = spawn(self, boost::bind(&https_ut::echo_session, _arg1), monitored);
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
      GCE_ERROR(lg) << ex.what();
    }
  }

  static void handle_http(ssl_socket_t& socket, std::string const& http_req)
  {
    boost::asio::streambuf request;
    std::ostream req_strm(&request);
    req_strm << http_req;

    // Send the request.
    boost::asio::write(socket, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;
    boost::asio::read_until(socket, response, "\r\n");

    // Check that response is OK.
    std::istream response_stream(&response);
    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/")
    {
      throw std::runtime_error("Invalid response");
    }
    if (status_code != 200)
    {
      std::stringstream ss;
      ss << "Response returned with status code " << status_code << "\n";
      throw std::runtime_error(ss.str());
    }

    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
    {
      //std::cout << header << "\n";
    }
    //std::cout << "\n";

    // Write whatever content we already have to output.
    if (response.size() > 0)
    {
      //std::cout << &response;
    }

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    if (boost::asio::read(socket, response,
          boost::asio::transfer_at_least(1), error))
    {
      //std::cout << &response;
    }
    if (error && error != boost::asio::error::eof)
      throw boost::system::system_error(error);

    //std::cout << std::endl;
  }

  static void handle_http_pipeline(ssl_socket_t& socket, std::string const& http_req)
  {
    boost::asio::streambuf request;
    std::ostream req_strm(&request);
    req_strm << http_req;

    // Send the request.
    boost::asio::write(socket, request);

    // Read the response status line. The response streambuf will automatically
    // grow to accommodate the entire line. The growth may be limited by passing
    // a maximum size to the streambuf constructor.
    boost::asio::streambuf response;

    // Read until EOF, writing data to output as we go.
    boost::system::error_code error;
    while (boost::asio::read(socket, response,
      boost::asio::transfer_at_least(1), error))
    {
      //std::cout << &response;
    }
    if (error && error != boost::asio::error::eof)
      throw boost::system::system_error(error);

    //std::cout << std::endl;
  }

  static void http_client(
    log::logger_t lg, 
    boost::shared_ptr<boost::asio::ssl::context> ssl_ctx, 
    bool err
    )
  {
    try
    {
      boost::asio::io_service ios;

      // Get a list of endpoints corresponding to the server name.
      tcp_resolver_t resolver(ios);
      tcp_resolver_t::query query("127.0.0.1", "23333");
      tcp_resolver_t::iterator eitr = resolver.resolve(query);

      boost::asio::ssl::stream<tcp_socket_t> ssl_skt(ios, *ssl_ctx);
      ssl_skt.set_verify_mode(boost::asio::ssl::verify_peer);
      ssl_skt.set_verify_callback(
          boost::bind(&https_ut::verify_certificate, _arg1, _arg2, lg));

      boost::asio::connect(ssl_skt.lowest_layer(), eitr);
      ssl_skt.handshake(boost::asio::ssl::stream_base::client);

      // Form the request. We specify the "Connection: close" header so that the
      // server will close the socket after transmitting the response. This will
      // allow us to treat all data up until the EOF as the content.
      handle_http(ssl_skt, https_post_raw);
      handle_http(ssl_skt, https_trunk_head);
      //handle_http(socket, https_post_bye);

      std::string http_req = https_get_raw;
      http_req += https_post_raw;
      http_req += https_trunk_head;
      if (err)
      {
        http_req += https_wrong_trunk_head;
      }
      else
      {
        http_req += https_post_bye;
      }
      handle_http_pipeline(ssl_skt, http_req);

      errcode_t ignored_ec;
      ssl_skt.shutdown(ignored_ec);
      GCE_INFO(lg) << "Https client end.";
    }
    catch (std::exception& e)
    {
      GCE_ERROR(lg) << "Exception: " << e.what();
    }
  }

  static void test_base()
  {
    log::asio_logger lgr;
    log::logger_t lg = boost::bind(&gce::log::asio_logger::output, &lgr, _arg1, "");

    try
    {
      size_t cln_count = boost::thread::hardware_concurrency();
      //size_t cln_count = 1;
      errcode_t ec;
      attributes attrs;
      attrs.lg_ = lg;
      context ctx_svr(attrs);

      threaded_actor base_svr = spawn(ctx_svr);

      aid_t svr = spawn(base_svr, boost::bind(&https_ut::echo_server, _arg1), monitored);
      base_svr->send(svr, "init");
      base_svr->recv("ready");

      boost::shared_ptr<boost::asio::ssl::context> ssl_ctx = 
        boost::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
      ssl_ctx->load_verify_file("test_https/ca.pem");

      boost::thread_group http_clients;
      for (size_t i=0; i<cln_count; ++i)
      {
        bool err = false;
        if (i % 2 == 0)
        {
          err = true;
        }
        http_clients.create_thread(boost::bind(&https_ut::http_client, lg, ssl_ctx, err));
      }

      http_clients.join_all();

      base_svr->send(svr, "end");
      base_svr->recv(exit);
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg) << ex.what();
    }
  }
};
}
}
