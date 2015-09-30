///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_SESSION_HPP
#define GCE_MYSQL_SESSION_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/session_fwd.hpp>
#include <gce/mysql/conn.hpp>
#include <gce/mysql/detail/conn_impl.hpp>

namespace gce
{
namespace mysql
{
class session
  : public addon_t
{
  typedef addon_t base_t;
  typedef session self_t;
  typedef base_t::scope<self_t, gce::detail::handler_allocator_t> scope_t;
  typedef scope_t::guard_ptr guard_ptr;

public:
  template <typename Actor>
  session(Actor a, conn_t const& c, snid_t const& snid = snid_nil)
    : base_t(a)
    , snid_(snid)
    , disposed_(false)
    , closed_(true)
    , opening_(false)
    , querying_(false)
    , pinging_(false)
    , c_(c)
    , ci_(get_conn_impl(c_))
    , qry_idx_(size_nil)
    , qry_(0)
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
      ci_->snd_.post(handle_close_binder(ci_, qry_idx_));
      scp_.notify();
    }
  }

public:
  /// open session, check if conn is on, if not try to connect, will return sn_open msg
  void open()
  {
    GCE_ASSERT(!opening_);
    opening_ = true;
    ci_->snd_.post(handle_open_binder(base_t::get_strand(), scp_.get(), ci_, qry_idx_, snid_));
  }

  /// execute sql(s), will return sn_query msg
  void execute(std::string const& qry, result_ptr res = result_ptr())
  {
    GCE_ASSERT(qry_ != 0).msg("please do open before execute");
    GCE_ASSERT(!querying_);
    if (&qry != qry_)
    {
      *qry_ = qry;
    }
    if (res)
    {
      res->clear();
    }
    querying_ = true;
    ci_->snd_.post(handle_execute_binder(base_t::get_strand(), scp_.get(), ci_, *qry_, res, snid_));
  }

  /// check if conn is on, if not try to connect, will return sn_ping msg
  void ping()
  {
    GCE_ASSERT(!pinging_);
    pinging_ = true;
    ci_->snd_.post(handle_ping_binder(base_t::get_strand(), scp_.get(), ci_, snid_));
  }

  std::string& get_query_buffer()
  {
    GCE_ASSERT(qry_ != 0);
    return *qry_;
  }

  snid_t const& get_snid() const
  {
    return snid_;
  }

private:
  ///----------------------------------------------------------------------------
  /// open
  ///----------------------------------------------------------------------------
  struct handle_open_binder
  {
    handle_open_binder(strand_t snd, guard_ptr guard, detail::conn_impl* ci, size_t qry_idx, snid_t const& snid)
      : snd_(snd)
      , guard_(guard)
      , ci_(ci)
      , qry_idx_(qry_idx)
      , snid_(snid)
    {
    }

    void operator()() const
    {
      self_t::handle_open(snd_, guard_, ci_, qry_idx_, snid_);
    }

    strand_t snd_;
    guard_ptr guard_;
    detail::conn_impl* ci_;
    size_t qry_idx_;
    snid_t const snid_;
  };

  static void handle_open(strand_t snd, guard_ptr guard, detail::conn_impl* ci, size_t qry_idx, snid_t const& snid)
  {
    message m(sn_open);
    if (snid != snid_nil)
    {
      m << snid;
    }

    std::pair<errno_t, std::string> ret = std::make_pair(errno_nil, std::string());
    if (ci->is_closed())
    {
      ret = ci->connect();
    }
    m << ret.first << ret.second;

    if (qry_idx == size_nil)
    {
      qry_idx = ci->query_buffer_list_.add(std::string());
    }
    std::string* qry_buf = ci->query_buffer_list_.get(qry_idx);
    GCE_ASSERT(qry_buf != 0);

    snd.post(end_open_binder(guard, m, qry_idx, qry_buf));
  }

  struct end_open_binder
  {
    end_open_binder(guard_ptr guard, message& msg, size_t qry_idx, std::string* qry_buf)
      : guard_(guard)
      , msg_(msg)
      , qry_idx_(qry_idx)
      , qry_buf_(qry_buf)
    {
    }

    void operator()() const
    {
      self_t::end_open(guard_, msg_, qry_idx_, qry_buf_);
    }

    guard_ptr guard_;
    message msg_;
    size_t qry_idx_;
    std::string* qry_buf_;
  };

  static void end_open(guard_ptr guard, message const& m, size_t qry_idx, std::string* qry_buf)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->opening_ = false;
    o->qry_idx_ = qry_idx;
    o->qry_ = qry_buf;
    o->send2actor(m);
  }
  ///----------------------------------------------------------------------------
  /// execute
  ///----------------------------------------------------------------------------
  struct handle_execute_binder
  {
    handle_execute_binder(
      strand_t snd, guard_ptr guard, detail::conn_impl* ci, 
      std::string const& qry, result_ptr res, snid_t const& snid
      )
      : snd_(snd)
      , guard_(guard)
      , ci_(ci)
      , qry_(qry)
      , res_(res)
      , snid_(snid)
    {
    }

    void operator()() const
    {
      self_t::handle_execute(snd_, guard_, ci_, qry_, res_, snid_);
    }

    strand_t snd_;
    guard_ptr guard_;
    detail::conn_impl* ci_;
    std::string const& qry_;
    result_ptr res_;
    snid_t const snid_;
  };

  static void handle_execute(
    strand_t snd, guard_ptr guard, detail::conn_impl* ci, 
    std::string const& qry, result_ptr res, snid_t const& snid
    )
  {
    message m(sn_query);
    if (snid != snid_nil)
    {
      m << snid;
    }

    std::pair<errno_t, std::string> ret = std::make_pair(errno_nil, std::string());
    //result_ptr res;
    if (ci->is_closed())
    {
      ret = ci->connect();
    }

    if (ret.first == errno_nil)
    {
      ret = ci->query(qry, res);
    }

    m << ret.first << ret.second << res;
    snd.post(end_execute_binder(guard, m));
  }

  struct end_execute_binder
  {
    end_execute_binder(guard_ptr guard, message& msg)
      : guard_(guard)
      , msg_(msg)
    {
    }

    void operator()() const
    {
      self_t::end_execute(guard_, msg_);
    }

    guard_ptr guard_;
    message msg_;
  };

  static void end_execute(guard_ptr guard, message const& m)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->querying_ = false;
    o->qry_->clear();
    o->send2actor(m);
  }
  ///----------------------------------------------------------------------------
  /// add
  ///----------------------------------------------------------------------------
  struct handle_add_binder
  {
    explicit handle_add_binder(detail::conn_impl* ci)
      : ci_(ci)
    {
    }

    void operator()() const
    {
      self_t::handle_add(ci_);
    }

    detail::conn_impl* ci_;
  };

  void add()
  {
    ci_->snd_.post(handle_add_binder(ci_));
  }

  static void handle_add(detail::conn_impl* ci)
  {
    ci->connect();
    ci->add_ref();
  }
  ///----------------------------------------------------------------------------
  /// ping
  ///----------------------------------------------------------------------------
  struct handle_ping_binder
  {
    handle_ping_binder(strand_t snd, guard_ptr guard, detail::conn_impl* ci, snid_t const& snid)
      : snd_(snd)
      , guard_(guard)
      , ci_(ci)
      , snid_(snid)
    {
    }

    void operator()() const
    {
      self_t::handle_ping(snd_, guard_, ci_, snid_);
    }

    strand_t snd_;
    guard_ptr guard_;
    detail::conn_impl* ci_;
    snid_t const snid_;
  };

  static void handle_ping(strand_t snd, guard_ptr guard, detail::conn_impl* ci, snid_t const& snid)
  {
    message m(sn_ping);
    if (snid != snid_nil)
    {
      m << snid;
    }

    std::pair<errno_t, std::string> ret = std::make_pair(errno_nil, std::string());
    if (ci->is_closed())
    {
      ret = ci->connect();
    }
    m << ret.first << ret.second;

    snd.post(end_ping_binder(guard, m));
  }

  struct end_ping_binder
  {
    end_ping_binder(guard_ptr guard, message& msg)
      : guard_(guard)
      , msg_(msg)
    {
    }

    void operator()() const
    {
      self_t::end_ping(guard_, msg_);
    }

    guard_ptr guard_;
    message msg_;
  };

  static void end_ping(guard_ptr guard, message const& m)
  {
    self_t* o = guard->get();
    if (!o)
    {
      return;
    }

    o->pinging_ = false;
    o->send2actor(m);
  }
  ///----------------------------------------------------------------------------
  /// close
  ///----------------------------------------------------------------------------
  struct handle_close_binder
  {
    explicit handle_close_binder(detail::conn_impl* ci, size_t qry_idx)
      : ci_(ci)
      , qry_idx_(qry_idx)
    {
    }

    void operator()() const
    {
      self_t::handle_close(ci_, qry_idx_);
    }

    detail::conn_impl* ci_;
    size_t qry_idx_;
  };

  static void handle_close(detail::conn_impl* ci, size_t qry_idx)
  {
    ci->query_buffer_list_.rmv(qry_idx);
    ci->sub_ref();
  }

private:
  snid_t const snid_;
  bool disposed_;
  bool closed_;
  volatile bool opening_;
  volatile bool querying_;
  volatile bool pinging_;
  conn_t c_;
  detail::conn_impl* ci_;

  size_t qry_idx_;
  std::string* qry_;

  /// for quit
  scope_t scp_;
};
}
}

#endif /// GCE_MYSQL_SESSION_HPP
