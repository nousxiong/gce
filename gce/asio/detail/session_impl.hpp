///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_DETAIL_SESSION_IMPL_HPP
#define GCE_ASIO_DETAIL_SESSION_IMPL_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/tcp/socket.hpp>
#include <gce/asio/sn_option.hpp>
#include <gce/asio/parser/basic.hpp>
#include <gce/asio/detail/session_fwd.hpp>
#include <gce/detail/buffer_ref.hpp>
#ifdef GCE_OPENSSL
# include <boost/asio/ssl.hpp>
# include <boost/mpl/if.hpp>
# include <boost/type_traits.hpp>
#endif

#ifndef GCE_SESSION_RECV_BUFFER_MIN_SIZE
# define GCE_SESSION_RECV_BUFFER_MIN_SIZE 60000
#endif

namespace gce
{
namespace asio
{
namespace detail
{
template <typename Parser, typename Socket>
class session_impl
{
};

template <typename Socket>
class basic_session_impl
{
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef Socket socket_t;
  typedef tcp::socket tcp_socket_t;
#ifdef GCE_OPENSSL
  typedef ssl::stream<> ssl_socket_t;
#endif

#ifdef GCE_OPENSSL
  typedef typename boost::mpl::if_<
    typename boost::is_same<socket_t, boost::asio::ip::tcp::socket>::type, tcp_socket_t, ssl_socket_t
    >::type socket;
#else
  typedef tcp_socket_t socket;
#endif

protected:
  enum status
  {
    ready = 0,
    on,
    off,
  };

public:
  basic_session_impl(
    resolver_t::iterator eitr, 
    snopt_t opt
    )
    : eitr_(eitr)
    , opt_(opt)
    , is_conn_(eitr != resolver_t::iterator())
    , bigmsg_size_(opt.bigmsg_size)
  {
  }

  ~basic_session_impl()
  {
    if (ex_)
    {
      send_close(ex_);
    }
  }

public:
  public:
  void run(stackless_actor self, boost::shared_ptr<socket_t> skt)
  {
    try
    {
      GCE_REENTER (self)
      {
        lg_ = self.get_context().get_logger();
        self_ = self;
        skt_ = boost::in_place(self_, skt);
        tmr_ = boost::in_place(self_);
        stat_ = ready;

        GCE_YIELD self_->recv(sire_, "init");
        while (true)
        {
          GCE_YIELD self_.recv(sender_, msg_);
          match_t type = msg_.get_type();
          if (type == api_open)
          {
            open();
          }
          else if (type == asio::tcp::as_conn)
          {
            errcode_t ec;
            msg_ >> ec;
            async_open(*skt_, ec);
          }
          else if (type == asio::ssl::as_handshake)
          {
            errcode_t ec;
            msg_ >> ec;
            handle_handshake(*skt_, ec);
          }
          else if (type == asio::ssl::as_shutdown)
          {
            errcode_t ec;
            msg_ >> ec;
            handle_shutdown(*skt_, ec);
          }
          else if (type == api_close)
          {
            bool grateful;
            msg_ >> grateful;
            close(grateful);
          }
          else if (type == as_timeout)
          {
            errcode_t ec;
            msg_ >> ec;
            handle_timeout(ec);
          }
          else if (type == asio::tcp::as_recv_some)
          {
            errcode_t ec;
            size_t bytes_transferred;
            msg_ >> ec >> bytes_transferred;
            handle_read(ec, bytes_transferred);
          }
          else if (type == asio::tcp::as_recv_until)
          {
            errcode_t ec;
            size_t bytes_transferred;
            msg_ >> ec >> bytes_transferred;
            handle_read(ec, bytes_transferred);
          }
          else if (type == asio::tcp::as_send)
          {
            errcode_t ec;
            size_t bytes_transferred;
            msg_ >> ec >> bytes_transferred;
            handle_send(ec, bytes_transferred);
          }
          else if (type == exit)
          {
            exited_ = true;
            if (stat_ != off)
            {
              close(false);
            }

            if (closed_)
            {
              break;
            }
          }
          else if (type == atom("exited"))
          {
            break;
          }
          else
          {
            /// api send
            send(msg_);
          }
        }
      }
    }
    catch (std::exception& ex)
    {
      GCE_ERROR(lg_) << "session_impl::run: " << ex.what();
    }
  }

protected:
  virtual void begin_open() = 0;
  virtual void end_open() = 0;
  virtual void on_send(message&, message&) = 0;
  virtual void begin_recv() = 0;
  virtual void handle_read(errcode_t, size_t) = 0;

protected:
  void open()
  {
    GCE_ASSERT(stat_ != on)(stat_);
    stat_ = ready;
    exited_ = false;
    grateful_ = true;
    closed_ = false;
    sending_ = false;
    recving_ = false;
    timing_ = false;
    shuting_down_ = false;
    shutdown_ = false;
    gather_buffer_.clear();
    send_que_.clear();
    ex_.clear();

    /// invoke child cb
    begin_open();

    if (is_conn_)
    {
      skt_->async_connect(eitr_);
      return;
    }
    else
    {
      async_open(*skt_, errcode_t());
    }
  }

  void async_open(tcp_socket_t&, errcode_t ec)
  {
    handle_open(ec);
  }

#ifdef GCE_OPENSSL
  void async_open(ssl_socket_t& s, errcode_t ec)
  {
    if (ec)
    {
      send_close(ec);
      return;
    }

    s.async_handshake(is_conn_ ? ssl_socket_t::impl_t::client : ssl_socket_t::impl_t::server);
  }
#endif

  void handle_handshake(tcp_socket_t&, errcode_t)
  {
  }

#ifdef GCE_OPENSSL
  void handle_handshake(ssl_socket_t&, errcode_t ec)
  {
    handle_open(ec);
  }
#endif

  void handle_open(errcode_t ec = errcode_t())
  {
    if (ec)
    {
      send_close(ec);
      return;
    }

    stat_ = on;
    expire();
    start_idle_timer();
    self_->send(sire_, asio::sn_open);

    /// invoke child cb
    end_open();

    begin_recv();
  }

  void expire()
  {
    (*tmr_)->expires_from_now(to_chrono(opt_.idle_period));
  }

  void start_idle_timer()
  {
    timing_ = true;
    tmr_->async_wait();
  }

  void handle_timeout(errcode_t ec)
  {
    timing_ = false;
    try_close(ec);

    if (stat_ != off)
    {
      if ((*tmr_)->expires_at() <= boost::chrono::system_clock::now())
      {
        self_->send(sire_, asio::sn_idle);
        expire();
      }
      start_idle_timer();
    }
  }

  void send(message& m)
  {
    if (stat_ != on)
    {
      return;
    }

    GCE_ASSERT(gather_buffer_.size() <= send_que_.size());
    if (gather_buffer_.size() == send_que_.size())
    {
      send_que_.push_back(msg_nil_);
    }

    message& msg = send_que_.back();
    /// invoke child cb
    on_send(msg, m);

    if (m.size() < bigmsg_size_)
    {
      msg << message::chunk(m.data(), m.size());
    }
    else
    {
      send_que_.push_back(m);
    }

    if (!is_sending())
    {
      begin_send();
    }
  }

  void begin_send()
  {
    sending_ = true;
    for (size_t i=0, size=(std::min)((size_t)GCE_IOV_MAX, send_que_.size()); i<size; ++i)
    {
      message const& msg = send_que_[i];
      gather_buffer_.push_back(boost::asio::buffer(msg.data(), msg.size()));
    }

    skt_->async_write(gather_buffer_);
  }

  void handle_send(errcode_t ec, size_t bytes_transferred)
  {
    sending_ = false;
    for (size_t i=0, size=gather_buffer_.size(); i<size; ++i)
    {
      send_que_.pop_front();
    }
    gather_buffer_.clear();

    if (!ec && !send_que_.empty())
    {
      begin_send();
    }
    else if (stat_ == off)
    {
      close_socket(*skt_);
    }

    try_close(ec);
  }

  bool is_sending() const
  {
    return !gather_buffer_.empty();
  }

  void close(bool grateful)
  {
    if (stat_ != off)
    {
      stat_ = off;
      grateful_ = grateful;
      errcode_t ignored_ec;
      (*tmr_)->cancel(ignored_ec);
      try_close();
    }
  }

  void try_close(errcode_t ec = errcode_t())
  {
    if (stat_ == off)
    {
      if (!grateful_)
      {
        close_socket(*skt_);
      }

      if (!sending_ && send_que_.empty())
      {
        close_socket(*skt_);
      }

      if (!sending_ && !timing_ && !recving_ && shutdown_)
      {
        close_socket(*skt_);
        send_close(ec);
      }
    }
  }

  void close_socket(tcp_socket_t& s)
  {
    errcode_t ignored_ec;
    s->close(ignored_ec);
    shutdown_ = true;
  }

  void close_socket(ssl_socket_t& s)
  {
    if (!shuting_down_)
    {
      shuting_down_ = true;
      s.async_shutdown();
    }
  }

  void handle_shutdown(tcp_socket_t&, errcode_t)
  {
  }

#ifdef GCE_OPENSSL
  void handle_shutdown(ssl_socket_t& s, errcode_t ec)
  {
    errcode_t ignored_ec;
    s->lowest_layer().close(ignored_ec);
    shutdown_ = true;
    try_close(ec);
  }
#endif

  void on_error(errcode_t const& ec)
  {
    close(ec ? false : true);
  }

  void send_close(errcode_t ec)
  {
    closed_ = true;
    self_->send(sire_, asio::sn_close, ec);
    if (exited_)
    {
      self_->send(self_.get_aid(), "exited");
    }
  }

protected:
  gce::log::logger_t lg_;
  stackless_actor self_;
  aid_t sire_;
  resolver_t::iterator eitr_;
  snopt_t const opt_;

  bool const is_conn_;
  boost::optional<socket> skt_;
  message const msg_nil_;

  /// temp, need clean
  status stat_;
  bool exited_;

  bool grateful_;
  bool closed_;
  bool sending_;
  bool recving_;
  bool timing_;
  bool shuting_down_;
  bool shutdown_;
  boost::optional<system_timer> tmr_;
  errcode_t ex_;

  std::deque<message> send_que_;
  std::vector<boost::asio::const_buffer> gather_buffer_;
  size_t const bigmsg_size_;

  /// temp local vars
  aid_t sender_;
  message msg_;
};
///----------------------------------------------------------------------------
/// recv_buffer
///----------------------------------------------------------------------------
struct recv_buffer
{
  recv_buffer()
    : data_((byte_t*)std::malloc(GCE_SESSION_RECV_BUFFER_MIN_SIZE))
    , size_(GCE_SESSION_RECV_BUFFER_MIN_SIZE)
  {
  }

  ~recv_buffer()
  {
    if (data_)
    {
      std::free(data_);
    }
  }

  void resize(size_t size)
  {
    if (size_ < size)
    {
      void* p = std::realloc(data_, size);
      if (!p)
      {
        throw std::bad_alloc();
      }
      data_ = (byte_t*)p;
    }

    if (size > GCE_SESSION_RECV_BUFFER_MIN_SIZE)
    {
      size_ = size;
    }
    else
    {
      size_ = GCE_SESSION_RECV_BUFFER_MIN_SIZE;
    }
  }

  void reset(size_t size)
  {
    if (data_)
    {
      std::free(data_);
      data_ = 0;
    }

    if (size < GCE_SESSION_RECV_BUFFER_MIN_SIZE)
    {
      size = GCE_SESSION_RECV_BUFFER_MIN_SIZE;
    }

    void* p = std::malloc(size);
    if (!p)
    {
      throw std::bad_alloc();
    }
    data_ = (byte_t*)p;
    size_ = size;
  }

  byte_t* data()
  {
    return data_;
  }

  size_t size() const
  {
    return size_;
  }

  byte_t* data_;
  size_t size_;
};
///----------------------------------------------------------------------------
/// tcp session_impl with parser::length
///----------------------------------------------------------------------------
template <typename Socket>
class session_impl<asio::parser::length, Socket>
  : public basic_session_impl<Socket>
{
  typedef basic_session_impl<Socket> base_t;
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef asio::parser::length parser_t;

public:
  session_impl(
    boost::shared_ptr<parser_t> parser, 
    resolver_t::iterator eitr, 
    snopt_t opt
    )
    : base_t(eitr, opt)
    , parser_(parser)
    , recv_cache_(recv_buffer_.data(), recv_buffer_.size())
    , max_header_size_(parser_->max_header_size())
    , copy_size_(max_header_size_ * 5)
    , tmp_(new byte_t[copy_size_])
  {
  }

  ~session_impl()
  {
  }

protected:
  void begin_open()
  {
    netmsg_ = base_t::msg_nil_;
    recv_cache_.clear();
    recving_header_ = true;
    recving_msg_ = false;
  }

  void end_open()
  {
    netmsg_.set_type(asio::sn_recv);
  }

  void on_send(message& msg, message& m)
  {
    parser_->on_send(msg, m);
  }

  void begin_recv()
  {
    while (true)
    {
      if (base_t::stat_ != base_t::off && !parse_message(netmsg_))
      {
        return begin_read();
      }

      if (base_t::stat_ == base_t::off)
      {
        return;
      }

      /// message parse success, send to user
      message m(netmsg_);
      netmsg_ = base_t::msg_nil_;
      base_t::self_.send(base_t::sire_, m);
      netmsg_.set_type(asio::sn_recv);
    }
  }

  void begin_read()
  {
    base_t::recving_ = true;
    base_t::skt_->async_read_some(
      boost::asio::buffer(
        recv_cache_.get_write_data(), 
        recv_cache_.remain_write_size()
        )
      );
  }

  void handle_read(errcode_t ec, size_t bytes_transferred)
  {
    base_t::recving_ = false;
    if (ec)
    {
      base_t::on_error(ec);
    }
    else
    {
      base_t::expire();
      recv_cache_.write(bytes_transferred);
      begin_recv();
    }
    base_t::try_close(ec);
  }

  void on_error(errcode_t const& ec)
  {
    base_t::close(ec ? false : true);
  }

  void adjust_recv_buffer(size_t expect_size)
  {
    size_t const remain_size = recv_cache_.remain_read_size();
    if (remain_size <= copy_size_)
    {
      /// copy unread data to recv buffer's head, if total size > buffer size reset it
      size_t const copy_size = remain_size;
      if (expect_size <= recv_buffer_.size())
      {
        std::memmove(recv_buffer_.data(), recv_cache_.get_read_data(), copy_size);
      }
      else
      {
        std::memcpy(tmp_.get(), recv_cache_.get_read_data(), copy_size);
        recv_buffer_.reset(expect_size);
        std::memcpy(recv_buffer_.data(), tmp_.get(), copy_size);
      }
      recv_cache_.clear();
      recv_cache_.write(copy_size);
    }
    else
    {
      /// extend recv buffer
      size_t const new_size = recv_cache_.read_size() + expect_size;
      GCE_ASSERT(new_size >= recv_cache_.write_size())(new_size)(recv_cache_.write_size());
      recv_buffer_.resize(new_size);
      recv_cache_.reset(recv_buffer_.data(), recv_buffer_.size());
    }
  }

  bool parse_message(message& msg)
  {
    if (recving_header_)
    {
      byte_t* data = recv_cache_.get_read_data();
      size_t const remain_size = recv_cache_.remain_read_size();

      curr_hdr_ = parser_->on_recv_header(boost::asio::buffer(data, remain_size), msg);
      if (!curr_hdr_.finish_)
      {
        //pkr_.clear();
        if (recv_cache_.size() - recv_cache_.read_size() < max_header_size_)
        {
          adjust_recv_buffer(max_header_size_);
        }
        return false;
      }
      recving_header_ = false;
      recv_cache_.read(curr_hdr_.header_size_);
    }

    size_t const remain_size = recv_cache_.remain_read_size();
    if (remain_size < curr_hdr_.body_size_)
    {
      if (!recving_msg_)
      {
        if (curr_hdr_.body_size_ - remain_size >= GCE_SOCKET_BIG_MSG_SIZE)
        {
          /// change msg to large msg
          msg.to_large(curr_hdr_.body_size_ + max_header_size_);

          /// copy already recved data to msg
          msg << message::chunk(recv_cache_.get_read_data(), remain_size);

          /// reset recv_cache_ to msg buffer
          recv_cache_.clear();
          recv_cache_.reset(const_cast<byte_t*>(msg.data()), curr_hdr_.body_size_ + max_header_size_);
          recv_cache_.write(remain_size);

          /// prepare for writing last data
          pre_write_size_ = curr_hdr_.body_size_ - remain_size;
          msg.pre_write(pre_write_size_);

          /// switch to recving_msg_ mode
          recving_msg_ = true;
        }
      }

      if (!recving_msg_)
      {
        if (recv_cache_.size() - recv_cache_.read_size() < curr_hdr_.body_size_)
        {
          adjust_recv_buffer(curr_hdr_.body_size_);
        }
      }
      return false;
    }

    if (!recving_msg_)
    {
      byte_t* data = recv_cache_.get_read_data();
      recv_cache_.read(curr_hdr_.body_size_);
      msg << message::chunk(data, curr_hdr_.body_size_);
    }
    else
    {
      packer& pkr = msg.get_packer();
      pkr.skip_write(pre_write_size_);
      msg.end_write();

      recv_cache_.read(curr_hdr_.body_size_);
      byte_t* data = recv_cache_.get_read_data();
      size_t const remain_size = recv_cache_.remain_read_size();
      GCE_ASSERT(remain_size < recv_buffer_.size())(remain_size);

      recv_cache_.clear();
      recv_cache_.reset(recv_buffer_.data(), recv_buffer_.size());
      std::memcpy(recv_cache_.get_write_data(), data, remain_size);
      recv_cache_.write(remain_size);
      recving_msg_ = false;
    }
    recving_header_ = true;
    return true;
  }

private:
  boost::shared_ptr<parser_t> parser_;
  recv_buffer recv_buffer_;

  /// temp, need clean
  gce::detail::buffer_ref recv_cache_;
  bool recving_header_;
  bool recving_msg_;
  message netmsg_;

  /// temp header buffer
  size_t pre_write_size_;
  parser_t::header curr_hdr_;
  size_t const max_header_size_;
  size_t const copy_size_;
  boost::scoped_array<byte_t> tmp_;
};
///----------------------------------------------------------------------------
/// tcp session_impl with parser::regex
///----------------------------------------------------------------------------
template <typename Socket>
class session_impl<asio::parser::regex, Socket>
  : public basic_session_impl<Socket>
{
  typedef basic_session_impl<Socket> base_t;
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef asio::parser::regex parser_t;

public:
  session_impl(
    boost::shared_ptr<parser_t> parser, 
    resolver_t::iterator eitr, 
    snopt_t opt
    )
    : base_t(eitr, opt)
    , parser_(parser)
  {
  }

  ~session_impl()
  {
  }

protected:
  void begin_open()
  {
    b_.consume(b_.size());
  }

  void end_open()
  {
  }

  void on_send(message&, message&)
  {
  }

  void begin_recv()
  {
    if (base_t::stat_ == base_t::on)
    {
      begin_read();
    }
  }

  void begin_read()
  {
    base_t::recving_ = true;
    base_t::skt_->async_read_until(b_, parser_->get_expr());
  }

  void handle_read(errcode_t ec, size_t bytes_transferred)
  {
    base_t::recving_ = false;
    if (ec)
    {
      base_t::on_error(ec);
    }
    else
    {
      base_t::expire();

      message msg(asio::sn_recv);
      boost::asio::const_buffer buf = b_.data();
      msg << message::chunk(
        boost::asio::buffer_cast<byte_t const*>(buf), 
        bytes_transferred
        );
      b_.consume(bytes_transferred);
      parser_->on_recv(msg);

      /// message parse success, send to user
      base_t::self_.send(base_t::sire_, msg);

      /// go on recv
      begin_recv();
    }
    base_t::try_close(ec);
  }

private:
  boost::shared_ptr<parser_t> parser_;

  /// temp, need clean
  boost::asio::streambuf b_;
};
}
}
}

#endif /// GCE_ASIO_DETAIL_SESSION_IMPL_HPP
