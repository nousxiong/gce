///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_HTTP_CLIENT_CONNECTION_HPP
#define GCE_HTTP_CLIENT_CONNECTION_HPP

#include <gce/http/config.hpp>
#include <gce/http/request.hpp>
#include <gce/http/reply.hpp>
#include <gce/http/detail/reply_parser.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#ifdef GCE_OPENSSL
# include <boost/asio/ssl.hpp>
#endif

namespace gce
{
namespace http
{
namespace client
{
class connection
  : public addon_t
{
  enum
  {
    ha_recv = 0,
    ha_send,
    ha_close,

    ha_num,
  };
  
  typedef boost::asio::ip::tcp::socket tcp_socket_t;

#ifdef GCE_OPENSSL
  typedef boost::asio::ssl::stream<tcp_socket_t> ssl_socket_t;
#endif

  struct core
  {
    /// Socket.
    boost::shared_ptr<tcp_socket_t> tcp_skt_;
#ifdef GCE_OPENSSL
    boost::shared_ptr<ssl_socket_t> ssl_skt_;
#endif
    /// Recv buffer
    boost::array<char, GCE_HTTP_CLIENT_RECV_BUFFER_SIZE> recv_buffer_;
    /// The request to be sent to the server.
    std::deque<request_ptr> requests_;
    /// The incoming reply.
    std::deque<reply_ptr> replies_;

    /// Asio handler_allocator array.
    boost::array<gce::detail::handler_allocator_t, ha_num> ha_arr_;
  };

  typedef addon_t base_t;
  typedef connection self_t;
  typedef base_t::scope<self_t, core> scope_t;
  typedef scope_t::guard_ptr guard_ptr;

public:
  template <typename Actor>
  connection(Actor a, boost::shared_ptr<tcp_socket_t> skt)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , recving_(false)
    , sending_(false)
    , closing_(false)
    , closed_(false)
    , goon_(false)
    , tcp_(true)
    , scp_(this)
  {
    scp_.get()->get_attachment().tcp_skt_ = skt;
  }

#ifdef GCE_OPENSSL
  template <typename Actor>
  connection(Actor a, boost::shared_ptr<ssl_socket_t> skt)
    : addon_t(a)
    , snd_(base_t::get_strand())
    , recving_(false)
    , sending_(false)
    , closing_(false)
    , closed_(false)
    , goon_(false)
    , tcp_(false)
    , scp_(this)
  {
    scp_.get()->get_attachment().ssl_skt_ = skt;
  }
#endif

  ~connection()
  {
    if (!closing_)
    {
      dispose();
    }
  }

public:
  void recv(reply_ptr rep = reply_ptr())
  {
    GCE_ASSERT(!closing_);
    if (!parser_.is_upgrade())
    {
      GCE_ASSERT(!rep);
    }
    else
    {
      GCE_ASSERT(!recving_);
    }
    start_recv(rep);
  }

  void send(request_ptr req)
  {
    GCE_ASSERT(!!req);
    GCE_ASSERT(!closing_);
    scp_.get()->get_attachment().requests_.push_back(req);
    if (!sending_)
    {
      start_send();
    }
  }

  void close()
  {
    if (closing_)
    {
      return;
    }

    snd_.post(
      gce::detail::make_asio_alloc_handler(
        scp_.get()->get_attachment().ha_arr_[ha_close],
        boost::bind(
          &self_t::handle_close, scp_.get()
          )
        )
      );
    closing_ = true;
  }

  request_ptr make_request()
  {
    return boost::make_shared<request>();
  }

  /// For lua actor.
  void dispose()
  {
    scp_.notify();
    close_socket();
  }

private:
  void close_socket()
  {
    errcode_t ignored_ec;
    if (tcp_)
    {
      boost::shared_ptr<tcp_socket_t> skt = scp_.get()->get_attachment().tcp_skt_;
      skt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      skt->close(ignored_ec);
    }
    else
    {
#ifdef GCE_OPENSSL
      boost::shared_ptr<ssl_socket_t> skt = scp_.get()->get_attachment().ssl_skt_;
      skt->shutdown(ignored_ec);
      skt->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
      skt->lowest_layer().close(ignored_ec);
#endif
    }
  }

  void start_recv(reply_ptr rep)
  {
    std::deque<reply_ptr>& replies = scp_.get()->get_attachment().replies_;
    bool sent_rep = false;
    if (!replies.empty())
    {
      if (goon_ && replies.size() > 1)
      {
        send_reply();
        sent_rep = true;
      }
      else if (!goon_)
      {
        send_reply();
        sent_rep = true;
      }
    }

    if (!sent_rep && !recving_)
    {
      async_recv(rep);
    }
  }

  void async_recv(reply_ptr rep)
  {
    if (tcp_)
    {
      pri_async_recv(scp_.get()->get_attachment().tcp_skt_, rep);
    }
    else
    {
#ifdef GCE_OPENSSL
      pri_async_recv(scp_.get()->get_attachment().ssl_skt_, rep);
#endif
    }
    recving_ = true;
  }

  template <typename SocketPtr>
  void pri_async_recv(SocketPtr skt, reply_ptr rep)
  {
    skt->async_read_some(
      boost::asio::buffer(scp_.get()->get_attachment().recv_buffer_), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment().ha_arr_[ha_recv],
          boost::bind(
            &self_t::handle_recv, scp_.get(), rep, 
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
  }

  static void handle_recv(guard_ptr guard, reply_ptr rep, errcode_t const& ec, size_t bytes_transferred)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->pri_handle_recv(rep, ec, bytes_transferred);
  }

  void pri_handle_recv(reply_ptr rep, errcode_t const& ec, size_t bytes_transferred)
  {
    recving_ = false;
    if (!ec)
    {
      std::deque<reply_ptr>& replies = scp_.get()->get_attachment().replies_;
      boost::array<char, GCE_HTTP_CLIENT_RECV_BUFFER_SIZE> const& recv_buffer = scp_.get()->get_attachment().recv_buffer_;
      if (parser_.is_upgrade())
      {
        GCE_ASSERT(replies.empty());
        if (!!rep)
        {
          replies.push_back(rep);
        }
      }
      boost::tribool result = parser_.parse(replies, recv_buffer.data(), bytes_transferred);
      if (result || !result)
      {
        goon_ = false;
      }
      else
      {
        goon_ = true;
      }

      if (!goon_ || replies.size() > 1)
      {
        send_reply();
      }
      else if (!closing_ && goon_ && replies.size() == 1)
      {
        /// Continue recv.
        async_recv(rep);
      }
    }
    else
    {
      goon_ = false;
      send_reply(ec);
      if (closed_)
      {
        send_close();
      }
    }
  }

  void send_reply(errcode_t const& ec = errcode_t())
  {
    std::deque<reply_ptr>& replies = scp_.get()->get_attachment().replies_;
    recv_msg_.set_type(as_reply);
    recv_msg_ << ec;
    if (!ec && !replies.empty())
    {
      reply_ptr rep = replies.front();
      replies.pop_front();
      recv_msg_ << rep;
    }
    else
    {
      recv_msg_ << reply_ptr();
    }
    pri_send2actor(recv_msg_);
  }

  void start_send()
  {
    size_t size = to_send_buffers();
    if (size > 0)
    {
      if (tcp_)
      {
        async_send(*(scp_.get()->get_attachment().tcp_skt_), size);
      }
      else
      {
#ifdef GCE_OPENSSL
        async_send(*(scp_.get()->get_attachment().ssl_skt_), size);
#endif
      }
      sending_ = true;
    }
  }

  template <typename Socket>
  void async_send(Socket& skt, size_t size)
  {
    boost::asio::async_write(
      skt, send_buffers_, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment().ha_arr_[ha_send],
          boost::bind(
            &self_t::handle_send, scp_.get(), size, 
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
  }

  static void handle_send(guard_ptr guard, size_t size, errcode_t const& ec, size_t bytes_transferred)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->pri_handle_send(size, ec, bytes_transferred);
  }

  void pri_handle_send(size_t size, errcode_t const& ec, size_t bytes_transferred)
  {
    std::deque<request_ptr>& requests = scp_.get()->get_attachment().requests_;
    sending_ = false;
    if (!ec)
    {
      if (size == requests.size())
      {
        requests.clear();
      }
      else
      {
        for (size_t i=0; i<size; ++i)
        {
          requests.pop_front();
        }
        start_send();
      }
    }

    if (closing_ && !sending_)
    {
      pri_handle_close();
    }
  }

  /// Convert the request into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the request object must remain valid and
  /// not be changed until the write operation has completed.
  size_t to_send_buffers()
  {
    send_buffers_.clear();
    std::deque<request_ptr>& requests = scp_.get()->get_attachment().requests_;
    size_t size = requests.size();
    for (size_t i=0; i<size; ++i)
    {
      request_ptr req = requests[i];
      send_buffers_.push_back(boost::asio::buffer(req->method_));
      send_buffers_.push_back(boost::asio::buffer(misc_strings::space));
      send_buffers_.push_back(boost::asio::buffer(req->uri_));
      send_buffers_.push_back(boost::asio::buffer(misc_strings::space));
      send_buffers_.push_back(boost::asio::buffer(misc_strings::http_str));
      /// http version
      req->version_.assign(boost::lexical_cast<intbuf_t>(req->http_major_).cbegin());
      req->version_.push_back(*misc_strings::point);
      req->version_.append(boost::lexical_cast<intbuf_t>(req->http_minor_).cbegin());
      send_buffers_.push_back(boost::asio::buffer(req->version_));
      send_buffers_.push_back(boost::asio::buffer(misc_strings::crlf));
      for (size_t j = 0; j < req->headers_.size(); ++j)
      {
        header& h = req->headers_[j];
        send_buffers_.push_back(boost::asio::buffer(h.name_));
        send_buffers_.push_back(boost::asio::buffer(misc_strings::name_value_separator));
        send_buffers_.push_back(boost::asio::buffer(h.value_));
        send_buffers_.push_back(boost::asio::buffer(misc_strings::crlf));
      }
      send_buffers_.push_back(boost::asio::buffer(misc_strings::crlf));
      send_buffers_.push_back(boost::asio::buffer(req->content_.data(), req->content_.size()));
    }
    return size;
  }

  static void handle_close(guard_ptr guard)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    if (!o->sending_)
    {
      o->pri_handle_close();
    }
  }

  void pri_handle_close()
  {
    // Initiate graceful connection closure.
    close_socket();
    closed_ = true;
    if (!recving_)
    {
      send_close();
    }
  }

  void send_close()
  {
    close_msg_.set_type(as_close);
    pri_send2actor(close_msg_);
  }

private:
  void pri_send2actor(message& m)
  {
    message msg(m);
    m = msg_nil_;
    base_t::send2actor(msg);
  }

private:
  strand_t& snd_;

  /// Send buffers.
  std::vector<boost::asio::const_buffer> send_buffers_;

  /// Parser.
  detail::reply_parser parser_;

  /// Status.
  bool recving_;
  bool sending_;
  bool closing_;
  bool closed_;
  bool goon_;
  bool tcp_;
  message recv_msg_;
  message close_msg_;
  message const msg_nil_;

  /// for quit
  scope_t scp_;
};
} /// namespace server
} /// namespace http
} /// namespace gce

#endif /// GCE_HTTP_CLIENT_CONNECTION_HPP
