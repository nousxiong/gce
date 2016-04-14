///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_SESSION_HPP
#define GCE_REDIS_SESSION_HPP

#include <gce/redis/config.hpp>
#include <gce/redis/session_fwd.hpp>
#include <gce/redis/result.hpp>
#include <gce/redis/conn.hpp>
#include <gce/redis/detail/conn_impl.hpp>
#include <gce/resp/all.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/array.hpp>

namespace gce
{
namespace redis
{
class session
  : public addon_t
{
  typedef addon_t base_t;
  typedef session self_t;
  typedef base_t::scope<self_t> scope_t;
  typedef scope_t::guard_ptr guard_ptr;
  typedef detail::conn_impl<self_t> conn_impl_t;
  typedef detail::request<self_t> request_t;
  typedef boost::shared_ptr<request_t> request_ptr;
  typedef detail::response_handler<self_t> response_t;
  typedef std::queue<response_t> response_queue_t;
  typedef resp::encoder<resp::buffer> encoder_t;
  typedef encoder_t::command cmd_t;
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef boost::asio::ip::tcp::socket tcp_socket_t;

public:
  template <typename Actor>
  session(Actor a, conn_t const& c, snid_t const& snid = snid_nil)
    : base_t(a)
    , snd_(get_strand())
    , snid_(snid)
    , disposed_(false)
    , begin_cmd_(false)
    , opening_(false)
    , pinging_(false)
    , c_(c)
    , ci_(get_conn_impl<self_t>(c_))
    , sn_idx_(size_nil)
    , scp_(this)
  {
    add();
  }

  ~session()
  {
    dispose();
  }

  void dispose()
  {
    if (!disposed_)
    {
      disposed_ = true;
      ci_->snd_.post(handle_close_binder(ci_, sn_idx_));
      scp_.notify();
    }
  }

public:
  /// Open session, check if conn is on, if not try to connect, will return sn_open msg.
  void open()
  {
    GCE_ASSERT(!opening_);
    opening_ = true;
    ci_->snd_.post(handle_open_binder(snd_, scp_.get(), ci_, sn_idx_, sn_open));
  }

  void ping()
  {
    GCE_ASSERT(!pinging_);
    pinging_ = true;
    ci_->snd_.post(handle_open_binder(snd_, scp_.get(), ci_, sn_idx_, sn_ping));
  }

  ///----------------------------------------------------------------------------
  /// Query with args.
  ///----------------------------------------------------------------------------
  void query(boost::string_ref name)
  {
    cmd(name).execute();
  }

  template <typename A1>
  void query(boost::string_ref name, A1 const& a1)
  {
    cmd(name, a1).execute();
  }

  template <typename A1, typename A2>
  void query(boost::string_ref name, A1 const& a1, A2 const& a2)
  {
    cmd(name, a1, a2).execute();
  }

  template <typename A1, typename A2, typename A3>
  void query(boost::string_ref name, A1 const& a1, A2 const& a2, A3 const& a3)
  {
    cmd(name, a1, a2, a3).execute();
  }

  template <typename A1, typename A2, typename A3, typename A4>
  void query(boost::string_ref name, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    cmd(name, a1, a2, a3, a4).execute();
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  void query(boost::string_ref name, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    cmd(name, a1, a2, a3, a4, a5).execute();
  }

  ///----------------------------------------------------------------------------
  /// Set cmd with args.
  ///----------------------------------------------------------------------------
  self_t& cmd(boost::string_ref name)
  {
    if (!begin_cmd_)
    {
      begin_cmd_ = true;
      make_request();
      enc_.begin(curr_req_->resp_buffers_);
    }
    else
    {
      cmd_.end();
    }

    response_queue_t& responses = curr_req_->responses_;
    if (name == "subscribe" || name == "SUBSCRIBE" || name == "psubscribe" || name == "PSUBSCRIBE")
    {
      responses.push(response_t(snd_, scp_.get(), true));
    }
    else
    {
      responses.push(response_t(snd_, scp_.get()));
    }

    cmd_ = enc_.cmd(resp::buffer(name.data(), name.size()));
    return *this;
  }

  template <typename A1>
  self_t& cmd(boost::string_ref name, A1 const& a1)
  {
    cmd(name);
    arg(a1);
    return *this;
  }

  template <typename A1, typename A2>
  self_t& cmd(boost::string_ref name, A1 const& a1, A2 const& a2)
  {
    cmd(name);
    arg(a1);
    arg(a2);
    return *this;
  }

  template <typename A1, typename A2, typename A3>
  self_t& cmd(boost::string_ref name, A1 const& a1, A2 const& a2, A3 const& a3)
  {
    cmd(name);
    arg(a1);
    arg(a2);
    arg(a3);
    return *this;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  self_t& cmd(boost::string_ref name, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    cmd(name);
    arg(a1);
    arg(a2);
    arg(a3);
    arg(a4);
    return *this;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  self_t& cmd(boost::string_ref name, A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    cmd(name);
    arg(a1);
    arg(a2);
    arg(a3);
    arg(a4);
    arg(a5);
    return *this;
  }

  ///----------------------------------------------------------------------------
  /// Set args.
  ///----------------------------------------------------------------------------
  template <typename A1>
  self_t& args(A1 const& a1)
  {
    GCE_ASSERT(begin_cmd_);
    arg(a1);
    return *this;
  }

  template <typename A1, typename A2>
  self_t& args(A1 const& a1, A2 const& a2)
  {
    GCE_ASSERT(begin_cmd_);
    arg(a1);
    arg(a2);
    return *this;
  }

  template <typename A1, typename A2, typename A3>
  self_t& args(A1 const& a1, A2 const& a2, A3 const& a3)
  {
    GCE_ASSERT(begin_cmd_);
    arg(a1);
    arg(a2);
    arg(a3);
    return *this;
  }

  template <typename A1, typename A2, typename A3, typename A4>
  self_t& args(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4)
  {
    GCE_ASSERT(begin_cmd_);
    arg(a1);
    arg(a2);
    arg(a3);
    arg(a4);
    return *this;
  }

  template <typename A1, typename A2, typename A3, typename A4, typename A5>
  self_t& args(A1 const& a1, A2 const& a2, A3 const& a3, A4 const& a4, A5 const& a5)
  {
    GCE_ASSERT(begin_cmd_);
    arg(a1);
    arg(a2);
    arg(a3);
    arg(a4);
    arg(a5);
    return *this;
  }

  /// Execute cmd set before.
  void execute()
  {
    GCE_ASSERT(!opening_);
    GCE_ASSERT(!pinging_);
    GCE_ASSERT(begin_cmd_);
    GCE_ASSERT(sn_idx_ != size_nil).msg("before query, session must open");
    begin_cmd_ = false;

    /// send cmd(s)
    cmd_.end();
    enc_.end();

    std::vector<boost::asio::const_buffer>& send_buffers = curr_req_->send_buffers_;
    send_buffers.clear();
    BOOST_FOREACH(resp::buffer& buffer, curr_req_->resp_buffers_)
    {
      send_buffers.push_back(boost::asio::buffer(buffer.data(), buffer.size()));
    }

    curr_req_->used(true);
    ci_->snd_.post(query_handler(ci_, curr_req_));
    curr_req_.reset();
  }

  snid_t const& get_snid() const
  {
    return snid_;
  }

#ifdef GCE_LUA
  /// internal use
  struct luactx
  {
    luactx(cmd_t& cmd, request_ptr& curr_req)
      : cmd_(cmd)
      , curr_req_(curr_req)
    {
    }

    cmd_t& cmd_;
    request_ptr& curr_req_;
  };

  luactx get_luactx()
  {
    return luactx(cmd_, curr_req_);
  }
#endif

public:
  /// handle response
  static void handle_response(
    strand_t snd, 
    guard_ptr guard, 
    errcode_t const& ec, 
    resp::result& resp_res,
    std::string const& chan, 
    bool unsub
    )
  {
    result_ptr res = boost::make_shared<result>();
    res->set(resp_res);
    snd.post(end_query_binder(guard, res, ec, chan, unsub));
  }

private:
  ///----------------------------------------------------------------------------
  /// Args, if built-in type(e.g. int long), will be convert to string; 
  /// if not built-in type, using gce::packer serialize it.
  ///----------------------------------------------------------------------------
  template <class T>
  typename boost::enable_if<boost::is_arithmetic<T>, void>::type arg(T t)
  {
    GCE_ASSERT(gce::intbuf_t::static_size <= RESP_LARGE_BUFFER_SIZE);
    resp::buffer buffer;
    buffer.append(boost::lexical_cast<intbuf_t>(t).cbegin());
    cmd_.arg(buffer);
  }

  void arg(char const* str)
  {
    try_channel(str);

    size_t len = std::strlen(str);
    if (len <= RESP_LARGE_BUFFER_SIZE)
    {
      resp::buffer buffer;
      buffer.append(str);
      cmd_.arg(buffer);
    }
    else
    {
      message& large_arg = curr_req_->reserve_large_arg(len);
      byte_t const* buf = large_arg.reset_write(0);
      large_arg << message::chunk((byte_t const*)str, len);
      cmd_.arg(resp::buffer((char const*)buf, len));
    }
  }

  void arg(std::string const& str)
  {
    try_channel(str.c_str());

    size_t len = str.size();
    if (len <= RESP_LARGE_BUFFER_SIZE)
    {
      resp::buffer buffer;
      buffer.append(str);
      cmd_.arg(buffer);
    }
    else
    {
      message& large_arg = curr_req_->reserve_large_arg(len);
      byte_t const* buf = large_arg.reset_write(0);
      large_arg << message::chunk((byte_t const*)str.data(), len);
      cmd_.arg(resp::buffer((char const*)buf, len));
    }
  }

  template <class T>
  typename boost::enable_if_c<!boost::is_arithmetic<T>::value, void>::type arg(T const& t)
  {
    size_t len = packer::size_of(t);
    message& large_arg = curr_req_->reserve_large_arg(len);
    byte_t const* begin_buf = large_arg.reset_write(0);
    large_arg << t;
    byte_t const* end_buf = large_arg.reset_write(0);
    cmd_.arg(resp::buffer((char const*)begin_buf, end_buf - begin_buf));
  }

private:
  /// try set channel when is subscribe/unsubscribe
  void try_channel(char const* str)
  {
    response_queue_t& responses = curr_req_->responses_;
    if (!responses.empty())
    {
      response_t& h = responses.back();
      if (h.subscribe())
      {
        if (h.channel().empty())
        {
          h.channel_ = str;
        }
        else
        {
          responses.push(response_t(snd_, scp_.get(), h.subscribe(), str));
        }
      }
    }
  }

  ///----------------------------------------------------------------------------
  /// add
  ///----------------------------------------------------------------------------
  struct handle_add_binder
  {
    explicit handle_add_binder(conn_impl_t* ci, guard_ptr guard)
      : ci_(ci)
      , guard_(guard)
    {
    }

    void operator()() const
    {
      self_t::handle_add(ci_, guard_);
    }

    conn_impl_t* ci_;
    guard_ptr guard_;
  };

  void add()
  {
    ci_->snd_.post(handle_add_binder(ci_, scp_.get()));
  }

  static void handle_add(conn_impl_t* ci, guard_ptr guard)
  {
    ci->add_ref();
  }

  ///----------------------------------------------------------------------------
  /// open
  ///----------------------------------------------------------------------------
  struct handle_open_binder
  {
    handle_open_binder(strand_t snd, guard_ptr guard, conn_impl_t* ci, size_t sn_idx, match_t type)
      : snd_(snd)
      , guard_(guard)
      , ci_(ci)
      , sn_idx_(sn_idx)
      , type_(type)
    {
    }

    void operator()() const
    {
      self_t::handle_open(snd_, guard_, ci_, sn_idx_, type_);
    }

    strand_t snd_;
    guard_ptr guard_;
    conn_impl_t* ci_;
    size_t sn_idx_;
    match_t type_;
  };

  static void handle_open(strand_t snd, guard_ptr guard, conn_impl_t* ci, size_t sn_idx, match_t type)
  {
    if (!ci->is_open())
    {
      ci->async_connect(handle_connect_binder(snd, guard, ci, sn_idx, type));
    }
    else
    {
      /// already open, return directly
      sn_idx = ci->alloc_session_index(sn_idx);
      snd.post(end_open_binder(guard, errcode_t(), sn_idx, type));
    }
  }

  struct end_open_binder
  {
    end_open_binder(guard_ptr guard, errcode_t const& ec, size_t sn_idx, match_t type)
      : guard_(guard)
      , ec_(ec)
      , sn_idx_(sn_idx)
      , type_(type)
    {
    }

    void operator()() const
    {
      self_t::end_open(guard_, ec_, sn_idx_, type_);
    }

    guard_ptr guard_;
    errcode_t ec_;
    size_t sn_idx_;
    match_t type_;
  };

  static void end_open(guard_ptr guard, errcode_t const& ec, size_t sn_idx, match_t type)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    if (type == sn_open)
    {
      o->opening_ = false;
    }
    else
    {
      o->pinging_ = false;
    }
    o->sn_idx_ = sn_idx;

    message m(type);
    if (o->snid_ != snid_nil)
    {
      m << o->snid_;
    }

    errno_t errn = errno_nil;
    if (ec)
    {
      errn = make_errno(ec.value());
    }
    m << errn << ec.message();
    o->send2actor(m);
  }

  struct handle_connect_binder
  {
    handle_connect_binder(strand_t snd, guard_ptr guard, conn_impl_t* ci, size_t sn_idx, match_t const& type)
      : snd_(snd)
      , guard_(guard)
      , ci_(ci)
      , sn_idx_(sn_idx)
      , type_(type)
    {
    }

    void operator()(errcode_t const& ec) const
    {
      self_t::handle_connect(snd_, guard_, ci_, ec, sn_idx_, type_);
    }

    strand_t snd_;
    guard_ptr guard_;
    conn_impl_t* ci_;
    size_t sn_idx_;
    match_t const type_;
  };

  static void handle_connect(
    strand_t snd, 
    guard_ptr guard, 
    conn_impl_t* ci, 
    errcode_t const& ec,
    size_t sn_idx, 
    match_t const& type
    )
  {
    sn_idx = ci->alloc_session_index(sn_idx);
    snd.post(end_open_binder(guard, ec, sn_idx, type));
  }

  struct query_handler
  {
    query_handler(conn_impl_t* ci, request_ptr req)
      : ci_(ci)
      , req_(req)
    {
    }

    void operator()() const
    {
      ci_->async_request(req_);
    }

    conn_impl_t* ci_;
    request_ptr req_;
  };

  struct end_query_binder
  {
    end_query_binder(
      guard_ptr guard, 
      result_ptr res, 
      errcode_t const& ec, 
      std::string const& chan, 
      bool unsub
      )
      : guard_(guard)
      , res_(res)
      , ec_(ec)
      , chan_(chan)
      , unsub_(unsub)
    {
    }

    void operator()() const
    {
      self_t::end_query(guard_, res_, ec_, chan_, unsub_);
    }

    guard_ptr guard_;
    result_ptr res_;
    errcode_t ec_;
    std::string chan_;
    bool unsub_;
  };

  static void end_query(
    guard_ptr guard, 
    result_ptr res, 
    errcode_t const& ec, 
    std::string const& chan,
    bool unsub
    )
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    bool pub = !chan.empty();
    message m(pub && !unsub ? sn_pubmsg : sn_query);
    if (o->snid_ != snid_nil)
    {
      m << o->snid_;
    }

    if (m.get_type() == sn_pubmsg)
    {
      m << res;
    }
    else
    {
      errno_t errn = errno_nil;
      if (ec)
      {
        errn = make_errno(ec.value());
      }
      m << errn << ec.message() << res;
    }
    o->send2actor(m);
  }

  struct handle_close_binder
  {
    explicit handle_close_binder(conn_impl_t* ci, size_t sn_idx)
      : ci_(ci)
      , sn_idx_(sn_idx)
    {
    }

    void operator()() const
    {
      self_t::handle_close(ci_, sn_idx_);
    }

    conn_impl_t* ci_;
    size_t sn_idx_;
  };

  static void handle_close(conn_impl_t* ci, size_t sn_idx)
  {
    ci->sub_ref(sn_idx);
  }

  void make_request()
  {
    if (req_ && req_->used())
    {
      curr_req_ = boost::make_shared<request_t>(sn_idx_);
    }
    else
    {
      if (!req_)
      {
        req_ = boost::make_shared<request_t>(sn_idx_);
      }
      curr_req_ = req_;
    }
  }

private:
  strand_t& snd_;
  snid_t const snid_;

  /// status vars
  bool disposed_;
  bool begin_cmd_;
  bool opening_;
  bool pinging_;

  /// redis connection
  conn_t c_;
  conn_impl_t* ci_;
  size_t sn_idx_;

  /// resp encoder
  encoder_t enc_;
  request_ptr req_;
  request_ptr curr_req_;

  /// encode args buffer
  cmd_t cmd_;

  /// for quit
  scope_t scp_;
};
}
}

#endif /// GCE_REDIS_SESSION_HPP
