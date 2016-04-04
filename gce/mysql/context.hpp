///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_CONTEXT_HPP
#define GCE_MYSQL_CONTEXT_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/attributes.hpp>
#include <gce/mysql/context_id.hpp>
#include <gce/mysql/conn.hpp>
#include <gce/mysql/conn_option.hpp>
#include <gce/mysql/exception.hpp>
#include <gce/mysql/detail/conn_impl.hpp>
#include <gce/detail/unique_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/optional.hpp>
/// for compatibility with >= 1.59 (using boost::in_place)
#include <boost/utility/in_place_factory.hpp>
#include <boost/utility/typed_in_place_factory.hpp>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

namespace gce
{
namespace mysql
{
class context
{
public:
  explicit context(attributes const& attrs = attributes())
    : attrs_(attrs)
    , ctxid_(make_ctxid(this))
    , stopped_(false)
    , work_(boost::in_place(boost::ref(ios_)))
    , conn_list_(attrs_.thread_num_)
    , lg_(attrs_.lg_)
  {
    GCE_VERIFY(mysql_library_init(0, NULL, NULL) == 0)
      .msg("could not initialize MySQL library").except<init_exception>();
    attrs_.thread_begin_cb_list_.push_back(boost::bind(&context::begin_db_thread, _arg1));
    attrs_.thread_end_cb_list_.push_back(boost::bind(&context::end_db_thread, _arg1));

    for (size_t i=0; i<attrs_.thread_num_; ++i)
    {
      thread_group_.create_thread(
        boost::bind(
          &context::run, this, i,
          attrs_.thread_begin_cb_list_,
          attrs_.thread_end_cb_list_
          )
        );
    }
  }

  ~context()
  {
    stop();
  }

public:
  ctxid_t get_ctxid() const
  {
    return ctxid_;
  }

  conn_t make_conn(
    std::string const& host,
    uint16_t port,
    std::string const& usr,
    std::string const& pwd,
    std::string const& db,
    connopt_t const& opt
    )
  {
    GCE_ASSERT(!stopped_);
    conn_t c;
    detail::conn_impl* impl = new detail::conn_impl(ios_);
    impl->host_ = host;
    impl->port_ = port;
    impl->usr_ = usr;
    impl->pwd_ = pwd;
    impl->db_ = db;
    impl->opt_ = opt;
    conn_list_.push(impl);
    c.ptr_ = (uint64_t)impl;
    return c;
  }

private:
  static void begin_db_thread(thrid_t)
  {
    mysql_thread_init();
  }

  static void end_db_thread(thrid_t)
  {
    mysql_thread_end();
  }

  void run(
    thrid_t id,
    std::vector<thread_callback_t> const& begin_cb_list,
    std::vector<thread_callback_t> const& end_cb_list
    )
  {
    BOOST_FOREACH(thread_callback_t const& cb, begin_cb_list)
    {
      cb(id);
    }

    while (true)
    {
      try
      {
        ios_.run();
        break;
      }
      catch (...)
      {
        GCE_ERROR(lg_)(__FILE__)(__LINE__) <<
          boost::current_exception_diagnostic_information();
      }
    }

    BOOST_FOREACH(thread_callback_t const& cb, end_cb_list)
    {
      cb(id);
    }
  }

  void stop()
  {
    /// close all conns
    stopped_ = true;
    std::vector<detail::conn_impl*> delete_list;
    detail::conn_impl* ci = 0;
    while (conn_list_.pop(ci))
    {
      delete_list.push_back(ci);
      ci->snd_.post(boost::bind(&detail::conn_impl::disconnect, ci));
    }
    work_ = boost::none;
    thread_group_.join_all();

    for (size_t i=0, size=delete_list.size(); i<size; ++i)
    {
      delete delete_list[i];
    }

    mysql_library_end();
  }

private:
  attributes attrs_;
  ctxid_t ctxid_;
  volatile bool stopped_;

  io_service_t ios_;
  boost::optional<io_service_t::work> work_;
  boost::thread_group thread_group_;

  boost::lockfree::queue<detail::conn_impl*> conn_list_;

  log::logger_t lg_;
};

static conn_t make_conn(
  ctxid_t const& ctxid, 
  std::string const& host,
  uint16_t port,
  std::string const& usr,
  std::string const& pwd,
  std::string const& db,
  connopt_t const& opt = make_connopt()
  )
{
  context* ctx = get_context(ctxid);
  return ctx->make_conn(host, port, usr, pwd, db, opt);
}
}
}

#endif /// GCE_MYSQL_CONTEXT_HPP
