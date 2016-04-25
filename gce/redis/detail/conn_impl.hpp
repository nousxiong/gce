///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REIDS_DETAIL_CONN_IMPL_HPP
#define GCE_REIDS_DETAIL_CONN_IMPL_HPP

#include <gce/redis/config.hpp>
#include <gce/redis/result.hpp>
#include <gce/redis/errno.hpp>
#include <gce/resp/all.hpp>
#include <gce/format/all.hpp>
#include <gce/detail/index_table.hpp>
#include <gce/detail/timer.hpp>
#include <boost/asio.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <boost/array.hpp>
#include <algorithm>
#include <map>
#include <vector>
#include <queue>
#include <utility>

#define GCE_REDIS_RECV_BUFFER_SIZE 1024 * 16

namespace gce
{
namespace redis
{
namespace detail
{
template <typename Session>
struct response_handler
{
  typedef gce::addon_t::scope<Session> scope_t;
  typedef typename scope_t::guard_ptr guard_ptr;

  response_handler(strand_t& snd, guard_ptr guard, char const* channel = "")
    : snd_(snd)
    , guard_(guard)
    , subscribe_(false)
    , channel_(channel)
  {
  }

  response_handler(strand_t& snd, guard_ptr guard, bool subscribe, char const* channel = "")
    : snd_(snd)
    , guard_(guard)
    , subscribe_(subscribe)
    , channel_(channel)
  {
  }

  response_handler(response_handler const& other)
    : snd_(other.snd_)
    , guard_(other.guard_)
    , subscribe_(other.subscribe_)
    , channel_(other.channel_)
  {
  }

  void operator()(errcode_t const& ec, resp::result& resp_res, bool unsub) const
  {
    Session::handle_response(snd_, guard_, ec, resp_res, channel_, unsub);
  }

  bool subscribe() const
  {
    return subscribe_;
  }

  std::string const& channel() const
  {
    return channel_;
  }

  strand_t snd_;
  guard_ptr guard_;
  /// include (p)subscribe
  bool subscribe_;
  /// include pattern
  std::string channel_;
};

typedef std::vector<boost::asio::const_buffer> send_buffers_t;
template <typename Session>
struct request
{
  explicit request(size_t sn_idx)
    : sn_idx_(sn_idx)
    , curr_large_arg_(0)
    , used_(false)
  {
    large_args_.push_back(message());
  }

  bool used() const
  {
    return used_;
  }

  void used(bool flag)
  {
    if (flag)
    {
      GCE_ASSERT(!used_);
    }
    used_ = flag;
  }

  size_t session_index() const
  {
    return sn_idx_;
  }

  void clear()
  {
    send_buffers_.clear();
    /*large_args_buffer_.reset_read();
    large_args_buffer_.reset_write();*/
    //large_args_.clear();
    for (size_t i=0; i<large_args_.size(); ++i)
    {
      message& large_arg = large_args_[i];
      large_arg.reset_read();
      large_arg.reset_write();
    }
    curr_large_arg_ = 0;
    resp_buffers_.clear();
    used(false);
  }

  message& reserve_large_arg(size_t len)
  {
    len = len > GCE_SMALL_MSG_SIZE + 8 ? len : GCE_SMALL_MSG_SIZE + 8;
    message* large_arg = &large_args_[curr_large_arg_];
    if (large_arg->size() == 0)
    {
      large_arg->to_large(len);
    }
    else if (large_arg->capacity() - large_arg->size() < len)
    {
      message msg;
      msg.to_large(len);
      large_args_.push_back(msg);
      ++curr_large_arg_;
      large_arg = &large_args_[curr_large_arg_];
    }
    return *large_arg;
  }

  message& reserve_large_arg()
  {
    message* large_arg = &large_args_[curr_large_arg_];
    if (large_arg->size() > 0)
    {
      large_arg->to_large();
    }
    else
    {
      message msg;
      msg.to_large();
      large_args_.push_back(msg);
      ++curr_large_arg_;
      large_arg = &large_args_[curr_large_arg_];
    }
    return *large_arg;
  }

  size_t const sn_idx_;
  send_buffers_t send_buffers_;
  //message large_args_buffer_;
  std::deque<message> large_args_;
  size_t curr_large_arg_;
  std::vector<resp::buffer> resp_buffers_;
  std::queue<response_handler<Session> > responses_;
  volatile bool used_;
};

template <typename Session>
struct conn_impl
{
  typedef Session* session_ptr;
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef boost::asio::ip::tcp::socket tcp_socket_t;
  typedef gce::detail::timer timer_t;
  typedef gce::addon_t::scope<Session> scope_t;
  typedef request<Session> request_t;
  typedef boost::shared_ptr<request_t> request_ptr;
  typedef response_handler<Session> response_t;
  typedef typename scope_t::guard_ptr guard_ptr;
  typedef boost::function<void (errcode_t const&)> connect_t;
  typedef std::map<size_t, response_t> channel_response_list_t;
  typedef std::map<std::string, channel_response_list_t> channel_response_map_t;
  typedef std::queue<request_ptr> request_queue_t;
  typedef std::queue<response_t> response_queue_t;
  typedef gce::detail::index_table<session_ptr> session_list_t;
  typedef conn_impl self_t;

  struct request_buffer
  {
    void clear()
    {
      send_buffers_.clear();
      while (!request_queue_.empty())
      {
        request_queue_.front()->clear();
        request_queue_.pop();
      }
    }

    send_buffers_t send_buffers_;
    request_queue_t request_queue_;
  };

  conn_impl(
    io_service_t& ios, 
    std::string const& host, uint16_t port, 
    gce::duration_t timeout
    )
    : snd_(ios)
    , ep_(boost::asio::ip::address::from_string(host), port)
    , tmr_(snd_)
    , timeout_(timeout)
    , skt_(ios)
    , sending_buffer_(0)
    , holding_buffer_(1)
    , conning_(false)
    , sending_(false)
    , querying_(false)
    , timing_(false)
    , recv_buffer_(GCE_REDIS_RECV_BUFFER_SIZE)
    , ref_(0)
  {
  }

  conn_impl(
    io_service_t& ios, 
    resolver_t::iterator eitr,
    gce::duration_t timeout
    )
    : snd_(ios)
    , eitr_(eitr)
    , tmr_(snd_)
    , timeout_(timeout)
    , skt_(ios)
    , sending_buffer_(0)
    , holding_buffer_(1)
    , conning_(false)
    , sending_(false)
    , querying_(false)
    , timing_(false)
    , recv_buffer_(GCE_REDIS_RECV_BUFFER_SIZE)
    , ref_(0)
  {
  }

  void add_ref()
  {
    ++ref_;
  }

  void sub_ref(size_t sn_idx)
  {
    session_list_.rmv(sn_idx);
    if (--ref_ == 0)
    {
      close();
    }
  }

  bool is_open() const
  {
    return skt_.is_open();
  }

  bool conning() const
  {
    return conning_;
  }

  void conning(bool flag)
  {
    if (flag)
    {
      GCE_ASSERT(!conning_);
    }
    conning_ = flag;
  }

  bool sending() const
  {
    return sending_;
  }

  void sending(bool flag)
  {
    if (flag)
    {
      GCE_ASSERT(!sending_);
    }
    sending_ = flag;
  }

  bool querying() const
  {
    return querying_;
  }

  void querying(bool flag)
  {
    if (flag)
    {
      GCE_ASSERT(!querying_);
    }
    querying_ = flag;
  }

  bool timing() const
  {
    return timing_;
  }

  void timing(bool flag)
  {
    if (flag)
    {
      GCE_ASSERT(!timing_);
    }
    timing_ = flag;
  }

  void close()
  {
    tmr_.cancel();
    errcode_t ignored_ec;
    skt_.close(ignored_ec);
  }

  bool disposable()
  {
    if (timing() || conning() || querying() || sending())
    {
      close();
      return false;
    }
    return true;
  }

  template <typename Handler>
  void async_connect(Handler const& h)
  {
    errc_.clear();
    connect_queue_.push(h);
    if (conning())
    {
      return;
    }

    conning(true);
    async_wait();

    if (ep_.address().is_unspecified())
    {
      /// using eitr_ to connect
      boost::asio::async_connect(
        skt_, eitr_,
        snd_.wrap(
          gce::detail::make_asio_alloc_handler(
            conn_ha_, handle_connect_binder(this)
            )
          )
        );
    }
    else
    {
      /// using ep_ to connect
      skt_.async_connect(
        ep_, 
        snd_.wrap(
          gce::detail::make_asio_alloc_handler(
            conn_ha_, handle_connect_binder(this)
            )
          )
        );
    }
  }

private:
  struct handle_connect_binder
  {
    explicit handle_connect_binder(self_t* ci)
      : ci_(ci)
    {
    }

    void operator()(errcode_t const& ec) const
    {
      self_t::handle_connect(ci_, ec);
    }

    void operator()(errcode_t const& ec, resolver_t::iterator) const
    {
      self_t::handle_connect(ci_, ec);
    }

    self_t* ci_;
  };

  static void handle_connect(self_t* ci, errcode_t const& ec)
  {
    ci->conning(false);
    ci->tmr_.cancel();
    errcode_t errc = ci->tmr_.timed_out() ? boost::asio::error::timed_out : ec;
    ci->errc_ = errc;
    ci->dec_ = resp::decoder();
    while (!ci->connect_queue_.empty())
    {
      connect_t h = ci->connect_queue_.front();
      ci->connect_queue_.pop();
      h(errc);
    }

    if (!errc)
    {
      ci->skt_.set_option(boost::asio::socket_base::receive_buffer_size(640000));
      ci->skt_.set_option(boost::asio::socket_base::send_buffer_size(640000));
      ci->skt_.set_option(boost::asio::ip::tcp::no_delay(true));
      ci->async_response();
    }
  }

  void async_wait()
  {
    timing(true);
    tmr_.async_wait(gce::to_chrono(timeout_), handle_timeout_binder(this));
  }

  struct handle_timeout_binder
  {
    handle_timeout_binder(self_t* ci)
      : ci_(ci)
    {
    }

    void operator()(errcode_t const& ec) const
    {
      self_t::handle_timeout(ci_, ec);
    }

    self_t* ci_;
  };

  static void handle_timeout(self_t* ci, errcode_t const& ec)
  {
    ci->timing(false);
    if (ec == boost::asio::error::timed_out)
    {
      ci->close();
    }
  }

public:
  void async_request(request_ptr req)
  {
    GCE_ASSERT(!conning());

    response_queue_t& responses = req->responses_;
    while (!responses.empty())
    {
      response_t& h = responses.front();
      size_t sn_idx = req->session_index();
      if (h.subscribe())
      {
        add_channel_response(h, sn_idx);
      }

      /// clear channel for sub/psub
      h.channel_.clear();
      response_queue_.push(h);
      responses.pop();
    }

    if (errc_)
    {
      response_error(errc_);
      return;
    }

    size_t index = sending() ? holding_buffer_ : sending_buffer_;
    send_buffers_t& buffers = req->send_buffers_;

    request_buffer& reqbuf = request_buffer_arr_[index];
    send_buffers_t& send_buffers = reqbuf.send_buffers_;
    send_buffers.resize(send_buffers.size() + buffers.size());
    std::copy_backward(buffers.begin(), buffers.end(), send_buffers.end());
    reqbuf.request_queue_.push(req);

    if (!sending())
    {
      async_send();
    }
  }

private:
  struct handle_request_binder
  {
    explicit handle_request_binder(self_t* ci)
      : ci_(ci)
    {
    }

    void operator()(errcode_t const& ec, size_t bytes_transferred) const
    {
      self_t::handle_request(ci_, ec, bytes_transferred);
    }

    self_t* ci_;
  };

  static void handle_request(self_t* ci, errcode_t const& ec, size_t)
  {
    ci->sending(false);
    if (ec)
    {
      ci->querying(false);
      ci->clear_buffers();
      ci->close();
    }
    else
    {
      ci->swap_buffers();
      ci->async_send();
    }
  }

  void async_send()
  {
    GCE_ASSERT(!sending());
    if (request_buffer_arr_[sending_buffer_].send_buffers_.empty())
    {
      return;
    }

    if (!querying())
    {
      querying(true);
      if (!timing())
      {
        async_wait();
      }
    }

    sending(true);
    boost::asio::async_write(
      skt_, request_buffer_arr_[sending_buffer_].send_buffers_, 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          req_ha_, 
          handle_request_binder(this)
          )
        )
      );
  }

  void clear_buffers()
  {
    request_buffer_arr_[sending_buffer_].clear();
    request_buffer_arr_[holding_buffer_].clear();
  }

  void swap_buffers()
  {
    request_buffer_arr_[sending_buffer_].clear();
    std::swap(sending_buffer_, holding_buffer_);
  }

  void async_response(size_t offset_size = 0)
  {
    skt_.async_read_some(
      boost::asio::buffer(recv_buffer_.data() + offset_size, recv_buffer_.size() - offset_size), 
      snd_.wrap(
        gce::detail::make_asio_alloc_handler(
          resp_ha_, 
          response_binder(this)
          )
        )
      );
  }

  struct response_binder
  {
    explicit response_binder(self_t* ci)
      : ci_(ci)
    {
    }

    void operator()(errcode_t const& ec, size_t bytes_transferred) const
    {
      self_t::handle_response(ci_, ec, bytes_transferred);
    }

    self_t* ci_;
  };

  static void handle_response(self_t* ci, errcode_t const& ec, size_t bytes_transferred)
  {
    resp::result resp_res;
    errcode_t errc = ci->tmr_.timed_out() ? boost::asio::error::timed_out : ec;
    if (errc)
    {
      ci->response_error(errc);
      return;
    }

    /// try decode buffer
    size_t curr_decoded_size = 0;
    while (true)
    {
      resp_res = 
        ci->dec_.decode(
          (char const*)ci->recv_buffer_.data() + curr_decoded_size, 
          bytes_transferred - curr_decoded_size
          );

      if (resp_res == resp::incompleted)
      {
        /// Move last data to buffer head.
        curr_decoded_size += resp_res.size();
        size_t last_size = bytes_transferred - curr_decoded_size;
        std::memmove(
          ci->recv_buffer_.data(), 
          (char const*)ci->recv_buffer_.data() + curr_decoded_size, 
          last_size
          );
        ci->recv_buffer_.resize(last_size + GCE_REDIS_RECV_BUFFER_SIZE);
        ci->async_response(last_size);
        break;
      }
      else if (resp_res == resp::completed)
      {
        ci->tmr_.cancel();
        ci->querying(false);

        if (resp_res.value().type() == resp::ty_array)
        {
          resp::unique_array<resp::unique_value> const& arr = resp_res.value().array();
          if (arr.size() > 0 && arr[0].type() == resp::ty_bulkstr)
          {
            resp::buffer const& cmd = arr[0].bulkstr();
            bool unsub = (cmd == "unsubscribe" || cmd == "punsubscribe") && arr.size() == 3;
            if (
              (cmd == "message" && arr.size() == 3) ||
              (cmd == "pmessage" && arr.size() == 4) ||
              unsub
              )
            {
              resp::buffer const& channel = arr[1].bulkstr();
              channel_response_list_t const* channel_response_list = ci->get_channel_response_list(channel);
              if (channel_response_list != 0)
              {
                errcode_t ec;
                resp::result copy_res;
                BOOST_FOREACH(typename channel_response_list_t::value_type const& pr, *channel_response_list)
                {
                  resp::result::copy(copy_res, resp_res);
                  pr.second(ec, copy_res, unsub);
                }
              }

              if (unsub)
              {
                ci->rmv_channel_responses(channel);
              }

              /// continue next loop in while
              curr_decoded_size += resp_res.size();
              continue;
            }
          }
        }

        response_t h = ci->response_queue_.front();
        ci->response_queue_.pop();
        h(errc, resp_res, false);
        curr_decoded_size += resp_res.size();
      }
      else
      {
        /// error, close conn
        ci->close();
        ci->response_error(boost::asio::error::invalid_argument);
        break;
      }
    }
  }

  void response_error(errcode_t const& ec)
  {
    errc_ = ec;
    resp::result resp_res;
    tmr_.cancel();
    querying(false);
    while (!response_queue_.empty())
    {
      response_t h = response_queue_.front();
      response_queue_.pop();
      h(ec, resp_res, false);
    }
  }

public:
  size_t alloc_session_index(size_t sn_idx)
  {
    if (sn_idx == size_nil)
    {
      sn_idx = session_list_.add(0);
    }
    return sn_idx;
  }

  void add_channel_response(response_t const& h, size_t sn_idx)
  {
    GCE_ASSERT(h.subscribe());
    std::string const& chan = h.channel();
    std::pair<typename channel_response_map_t::iterator, bool> pr = 
      channel_response_map_.insert(std::make_pair(chan, dummy_list_));
    channel_response_list_t& channel_response_list = pr.first->second;
    channel_response_list.insert(std::make_pair(sn_idx, h));
  }

  void rmv_channel_responses(resp::buffer const& chan)
  {
    channel_response_map_.erase(std::string(chan.data(), chan.size()));
  }

  channel_response_list_t const* get_channel_response_list(resp::buffer const& channel) const
  {
    std::string name(channel.data(), channel.size());
    typename channel_response_map_t::const_iterator itr = channel_response_map_.find(name);
    if (itr != channel_response_map_.end())
    {
      return &itr->second;
    }
    return 0;
  }

public:
  strand_t snd_;

  /// redis server ep
  resolver_t::iterator eitr_;
  boost::asio::ip::tcp::endpoint ep_;
  gce::detail::handler_allocator_t conn_ha_;
  /// async connect queue
  std::queue<connect_t> connect_queue_;
  /// connect timer and timeout
  timer_t tmr_;
  gce::duration_t timeout_;
  
  /// socket
  tcp_socket_t skt_;
  /// sending buffers array
  boost::array<request_buffer, 2> request_buffer_arr_;
  size_t sending_buffer_;
  size_t holding_buffer_;
  gce::detail::handler_allocator_t req_ha_;
  /// async response queue
  std::queue<response_t> response_queue_;
  gce::detail::handler_allocator_t resp_ha_;
  
  /// status vars
  bool conning_;
  bool sending_;
  bool querying_;
  bool timing_;

  /// current error
  errcode_t errc_;

  /// RESP
  resp::decoder dec_;
  gce::detail::buffer_st recv_buffer_;

  /// channels
  channel_response_map_t channel_response_map_;
  channel_response_list_t dummy_list_;

  int ref_;
  session_list_t session_list_;
};
} /// namespace detail
} /// namespace redis
} /// namespace gce

#endif /// GCE_REIDS_DETAIL_CONN_IMPL_HPP
