///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_HTTP_SERVER_CONNECTION_HPP
#define GCE_HTTP_SERVER_CONNECTION_HPP

#include <gce/http/config.hpp>
#include <gce/http/request.hpp>
#include <gce/http/reply.hpp>
#include <gce/http/detail/request_parser.hpp>
#include <boost/array.hpp>
#include <boost/lexical_cast.hpp>
#ifdef GCE_OPENSSL
# include <boost/asio/ssl.hpp>
#endif

namespace gce
{
namespace http
{
static match_t const as_close = atom("http_close");
namespace server
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
    boost::array<char, GCE_HTTP_SERVER_RECV_BUFFER_SIZE> recv_buffer_;
    /// The incoming request.
    std::deque<request_ptr> requests_;
    /// The reply to be sent back to the client.
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
  void recv(request_ptr req = request_ptr())
  {
    GCE_ASSERT(!closing_);
    if (!parser_.is_upgrade())
    {
      GCE_ASSERT(!req);
    }
    else
    {
      GCE_ASSERT(!recving_);
    }
    start_recv(req);
  }

  void send(reply_ptr rep)
  {
    GCE_ASSERT(!!rep);
    GCE_ASSERT(!closing_);
    scp_.get()->get_attachment().replies_.push_back(rep);
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

  reply_ptr make_reply(reply::status stat)
  {
    return boost::make_shared<reply>(stat);
  }

  /// Get an stock reply.
  reply_ptr stock_reply(reply::status stat)
  {
    reply_ptr rep = make_reply(stat);
    rep->stat_ = stat;
    rep->headers_.resize(2);
    rep->headers_[0].name_ = "Content-Length";
    rep->headers_[0].value_ = boost::lexical_cast<std::string>(rep->content_.size());
    rep->headers_[1].name_ = "Content-Type";
    rep->headers_[1].value_ = "text/html";
    return rep;
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

  void start_recv(request_ptr req)
  {
    std::deque<request_ptr>& requests = scp_.get()->get_attachment().requests_;
    bool sent_req = false;
    if (!requests.empty())
    {
      if (goon_ && requests.size() > 1)
      {
        send_request();
        sent_req = true;
      }
      else if (!goon_)
      {
        send_request();
        sent_req = true;
      }
    }

    if (!sent_req && !recving_)
    {
      async_recv(req);
    }
  }

  void async_recv(request_ptr req)
  {
    if (tcp_)
    {
      pri_async_recv(scp_.get()->get_attachment().tcp_skt_, req);
    }
    else
    {
#ifdef GCE_OPENSSL
      pri_async_recv(scp_.get()->get_attachment().ssl_skt_, req);
#endif
    }
    recving_ = true;
  }

  template <typename SocketPtr>
  void pri_async_recv(SocketPtr skt, request_ptr req)
  {
    skt->async_read_some(
      boost::asio::buffer(scp_.get()->get_attachment().recv_buffer_), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          scp_.get()->get_attachment().ha_arr_[ha_recv],
          boost::bind(
            &self_t::handle_recv, scp_.get(), req, 
            boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred
            )
          )
        )
      );
  }

  static void handle_recv(guard_ptr guard, request_ptr req, errcode_t const& ec, size_t bytes_transferred)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->pri_handle_recv(req, ec, bytes_transferred);
  }

  void pri_handle_recv(request_ptr req, errcode_t const& ec, size_t bytes_transferred)
  {
    recving_ = false;
    if (!ec)
    {
      std::deque<request_ptr>& requests = scp_.get()->get_attachment().requests_;
      boost::array<char, GCE_HTTP_SERVER_RECV_BUFFER_SIZE> const& recv_buffer = scp_.get()->get_attachment().recv_buffer_;
      if (parser_.is_upgrade())
      {
        GCE_ASSERT(requests.empty());
        if (!!req)
        {
          requests.push_back(req);
        }
      }
      boost::tribool result = parser_.parse(requests, recv_buffer.data(), bytes_transferred);
      if (result || !result)
      {
        goon_ = false;
      }
      else
      {
        goon_ = true;
      }

      if (!goon_ || requests.size() > 1)
      {
        send_request();
      }
      else if (!closing_ && goon_ && requests.size() == 1)
      {
        /// Continue recv.
        async_recv(req);
      }
    }
    else
    {
      goon_ = false;
      send_request(ec);
      if (closed_)
      {
        send_close();
      }
    }
  }

  void send_request(errcode_t const& ec = errcode_t())
  {
    std::deque<request_ptr>& requests = scp_.get()->get_attachment().requests_;
    recv_msg_.set_type(as_request);
    recv_msg_ << ec;
    if (!ec && !requests.empty())
    {
      request_ptr req = requests.front();
      requests.pop_front();
      recv_msg_ << req;
    }
    else
    {
      recv_msg_ << request_ptr();
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
    std::deque<reply_ptr>& replies = scp_.get()->get_attachment().replies_;
    sending_ = false;
    if (!ec)
    {
      if (size == replies.size())
      {
        replies.clear();
      }
      else
      {
        for (size_t i=0; i<size; ++i)
        {
          replies.pop_front();
        }
        start_send();
      }
    }

    if (closing_ && !sending_)
    {
      pri_handle_close();
    }
  }

  /// Convert the reply into a vector of buffers. The buffers do not own the
  /// underlying memory blocks, therefore the reply object must remain valid and
  /// not be changed until the write operation has completed.
  size_t to_send_buffers()
  {
    send_buffers_.clear();
    std::deque<reply_ptr>& replies = scp_.get()->get_attachment().replies_;
    size_t size = replies.size();
    for (size_t i=0; i<size; ++i)
    {
      reply_ptr rep = replies[i];
      send_buffers_.push_back(status_strings::to_buffer(rep->stat_));
      for (size_t j = 0; j < rep->headers_.size(); ++j)
      {
        header& h = rep->headers_[j];
        send_buffers_.push_back(boost::asio::buffer(h.name_));
        send_buffers_.push_back(boost::asio::buffer(misc_strings::name_value_separator));
        send_buffers_.push_back(boost::asio::buffer(h.value_));
        send_buffers_.push_back(boost::asio::buffer(misc_strings::crlf));
      }
      send_buffers_.push_back(boost::asio::buffer(misc_strings::crlf));
      send_buffers_.push_back(boost::asio::buffer(rep->content_.data(), rep->content_.size()));
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
  detail::request_parser parser_;

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

#endif /// GCE_HTTP_SERVER_CONNECTION_HPP
