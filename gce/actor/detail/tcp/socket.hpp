///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_TCP_SOCKET_HPP
#define GCE_ACTOR_DETAIL_TCP_SOCKET_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/asio.hpp>
#include <gce/actor/message.hpp>
#include <gce/actor/detail/basic_socket.hpp>
#include <gce/detail/linked_queue.hpp>
#include <gce/detail/asio_alloc_handler.hpp>
#include <gce/detail/bytes.hpp>
#include <gce/integer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <string>
#include <deque>

#define GCE_SOCKET_BIG_MSG_SIZE GCE_SMALL_MSG_SIZE * GCE_SOCKET_BIG_MSG_SCALE

namespace gce
{
typedef std::basic_string<byte_t, std::char_traits<byte_t>, std::allocator<byte_t> > bytes_t;
namespace detail
{
namespace tcp
{
class socket
  : public basic_socket
{
public:
  explicit socket(io_service_t& ios)
    : snd_(0)
    , msg_pool_(0)
    , reso_(ios)
    , sock_(ios)
    , closed_(false)
    , reconn_(false)
    , sync_(ios)
    , waiting_end_(false)

  {
  }

  socket(io_service_t& ios, std::string const& host, std::string const& port)
    : snd_(0)
    , msg_pool_(0)
    , reso_(ios)
    , sock_(ios)
    , host_(host)
    , port_(port)
    , closed_(false)
    , reconn_(false)
    , sync_(ios)
    , waiting_end_(false)
  {
  }

  ~socket()
  {
    if (msg_pool_ != 0)
    {
      while (message* m = send_que_.pop())
      {
        msg_pool_->free(m);
      }
    }
    else
    {
      GCE_ASSERT(send_que_.empty());
    }
  }

public:
  std::string remote_endpoint()
  {
    errcode_t ec;
    std::string epstr;
    boost::asio::ip::tcp::endpoint rep = sock_.remote_endpoint(ec);
    if (!ec)
    {
      epstr += "tcp://";
      epstr += rep.address().to_string();
      epstr += ":";
      epstr += boost::lexical_cast<intbuf_t>(rep.port()).cbegin();
    }
    return epstr;
  }

  void init(strand_t& snd, msg_pool_t& msg_pool)
  {
    snd_ = &snd;
    msg_pool_ = &msg_pool;
    gather_buffer_.reserve(GCE_IOV_MAX);
  }

  void send(message* m)
  {
    if (!waiting_end_)
    {
      GCE_ASSERT(gather_buffer_.size() <= send_que_.size());
      if (gather_buffer_.size() == send_que_.size())
      {
        //send_que_.push_back(msg_nil_);
        send_que_.push(msg_pool_->get());
      }

      header_t hdr = make_header((uint32_t)m->size(), m->get_type(), m->get_tag_offset());
      message* msg = send_que_.back();
      *msg << hdr;
      if (m->size() < GCE_SOCKET_BIG_MSG_SIZE)
      {
        *msg << message::chunk(m->data(), m->size());
        msg_pool_->free(m);
      }
      else
      {
        send_que_.push(m);
      }

      if (!is_sending())
      {
        begin_send();
      }
    }
  }

  size_t recv(byte_t* buf, size_t size, yielder ylder)
  {
    size_t len = 0;
    sock_.async_read_some(boost::asio::buffer(buf, size), ylder[len]);
    ylder.yield();
    return len;
  }

  void connect(yielder ylder)
  {
    if (is_sending())
    {
      reconn_ = true;
    }
    close_socket();
    boost::asio::ip::tcp::resolver::query query(host_, port_);
    boost::asio::ip::tcp::resolver::iterator itr;
    reso_.async_resolve(query, ylder[itr]);
    ylder.yield();
    if (ylder.ec_ && *ylder.ec_)
    {
      return;
    }
    boost::asio::async_connect(sock_, itr, ylder);
    ylder.yield();
  }

  void close()
  {
    closed_ = true;
    if (!is_sending())
    {
      close_socket();
    }
  }

  void wait_end(yielder ylder)
  {
    waiting_end_ = true;
    if (is_sending())
    {
      errcode_t ec;
      sync_.expires_from_now(to_chrono(infin));
      sync_.async_wait(ylder[ec]);
      ylder.yield();
    }
  }

  void reset()
  {
    close_socket();
  }

  boost::asio::ip::tcp::socket& get_socket()
  { 
    return sock_;
  }

private:
  bool is_sending() const
  {
    return !gather_buffer_.empty();
  }

  void close_socket()
  {
    errcode_t ignore_ec;
    sock_.close(ignore_ec);
  }

  void begin_send()
  {
    message* itr = send_que_.front();
    for (size_t i=0, size=(std::min)((size_t)GCE_IOV_MAX, send_que_.size()); i<size; ++i, itr=itr->next_)
    {
      //message const& msg = send_que_[i];
      GCE_ASSERT(itr != 0)(i);
      gather_buffer_.push_back(boost::asio::buffer(itr->data(), itr->size()));
    }
    strand_t& snd = *snd_;

    boost::asio::async_write(
      sock_,
      gather_buffer_,
      snd.wrap(
        make_asio_alloc_handler(
          ha_,
          end_send_binder(*this)
          /*boost::bind(
            &socket::end_send, this,
            boost::asio::placeholders::error
            )*/
          )
        )
      );
  }

  struct end_send_binder
  {
    explicit end_send_binder(socket& skt)
      : skt_(skt)
    {
    }

    void operator()(errcode_t const& ec, size_t) const
    {
      skt_.end_send(ec);
    }

    socket& skt_;
  };

  void end_send(errcode_t const& errc)
  {
    for (size_t i=0, size=gather_buffer_.size(); i<size; ++i)
    {
      //send_que_.pop_front();
      message* msg = send_que_.pop();
      GCE_ASSERT(msg != 0)(i);
      msg_pool_->free(msg);
    }
    gather_buffer_.clear();

    if (!errc && !send_que_.empty())
    {
      begin_send();
    }
    else if (closed_)
    {
      close();
    }

    if (closed_ && !is_sending())
    {
      errcode_t ignore_ec;
      sync_.cancel(ignore_ec);
    }

    reconn_ = false;
  }

private:
  strand_t* snd_;
  msg_pool_t* msg_pool_;
  boost::asio::ip::tcp::resolver reso_;
  boost::asio::ip::tcp::socket sock_;
  std::string const host_;
  std::string const port_;

  bool closed_;
  bool reconn_;

  //std::deque<message> send_que_;
  linked_queue<message> send_que_;
  std::vector<boost::asio::const_buffer> gather_buffer_;
  //message const msg_nil_;

  timer_t sync_;
  bool waiting_end_;

  handler_allocator_t ha_;
};
typedef boost::shared_ptr<socket> socket_ptr;
} /// namespace tcp
} /// namespace detail
} /// namespace gce

#endif /// GCE_ACTOR_DETAIL_TCP_SOCKET_HPP
