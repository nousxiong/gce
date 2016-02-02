///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_DETAIL_CONN_IMPL_HPP
#define GCE_MYSQL_DETAIL_CONN_IMPL_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/result.hpp>
#include <gce/mysql/conn_option.hpp>
#include <gce/mysql/errno.hpp>
#include <gce/format/all.hpp>
#include <gce/detail/index_table.hpp>
#include <boost/shared_ptr.hpp>

namespace gce
{
namespace mysql
{
namespace detail
{
struct conn_impl
{
  explicit conn_impl(io_service_t& ios)
    : snd_(ios)
    , mysql_(NULL)
    , ref_(0)
  {
  }

  void add_ref()
  {
    ++ref_;
  }

  void sub_ref()
  {
    if (--ref_ == 0)
    {
      disconnect();
    }
  }

  bool is_closed() const
  {
    return mysql_ == NULL;
  }

  std::pair<errno_t, std::string> connect()
  {
    std::string errmsg;
    if (mysql_ != NULL)
    {
      return std::make_pair(errno_nil, std::string());
    }

    mysql_ = mysql_init(NULL);
    GCE_ASSERT(mysql_ != NULL);

    if (!opt_.init_command.empty())
    {
      if (mysql_options(mysql_, MYSQL_INIT_COMMAND, opt_.init_command.c_str()) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (opt_.compress != -1)
    {
      if (mysql_options(mysql_, MYSQL_OPT_COMPRESS, NULL) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (opt_.connect_timeout != -1)
    {
      if (mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT, (unsigned int*)&opt_.connect_timeout) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (opt_.read_timeout != -1)
    {
      if (mysql_options(mysql_, MYSQL_OPT_READ_TIMEOUT, (unsigned int*)&opt_.read_timeout) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (opt_.reconnect != -1)
    {
      if (mysql_options(mysql_, MYSQL_OPT_RECONNECT, (my_bool*)&opt_.reconnect) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (opt_.write_timeout != -1)
    {
      if (mysql_options(mysql_, MYSQL_OPT_WRITE_TIMEOUT, (unsigned int*)&opt_.write_timeout) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (!opt_.read_default_file.empty())
    {
      if (mysql_options(mysql_, MYSQL_READ_DEFAULT_FILE, opt_.read_default_file.c_str()) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (!opt_.read_default_group.empty())
    {
      if (mysql_options(mysql_, MYSQL_READ_DEFAULT_GROUP, opt_.read_default_group.c_str()) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    if (!opt_.set_charset_name.empty())
    {
      if (mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, opt_.set_charset_name.c_str()) != 0)
      {
        return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
      }
    }

    /// set clientflag
    unsigned long clientflag = 0;
    if (opt_.client_compress != -1)
    {
      clientflag |= CLIENT_COMPRESS;
    }

    if (opt_.client_found_rows != -1)
    {
      clientflag |= CLIENT_FOUND_ROWS;
    }

    if (opt_.client_ignore_sigpipe != -1)
    {
      clientflag |= CLIENT_IGNORE_SIGPIPE;
    }

    if (opt_.client_ignore_space != -1)
    {
      clientflag |= CLIENT_IGNORE_SPACE;
    }

    if (opt_.client_multi_results != -1)
    {
      clientflag |= CLIENT_MULTI_RESULTS;
    }

    if (opt_.client_multi_statements != -1)
    {
      clientflag |= CLIENT_MULTI_STATEMENTS;
    }

    if (
      mysql_real_connect(
        mysql_, host_.c_str(), usr_.c_str(), pwd_.c_str(), 
        db_.c_str(), port_, NULL, clientflag
        ) == NULL
      )
    {
      return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
    }

    return std::make_pair(errno_nil, std::string());
  }

  std::pair<errno_t, std::string> query(fmt::MemoryWriter const& qry, result_ptr& res)
  {
    if (mysql_real_query(mysql_, qry.data(), qry.size()) != 0)
    {
      return std::make_pair(make_errno(mysql_errno(mysql_)), mysql_error(mysql_));
    }

    /// get result
    int status = 0;
    errno_t errn = errno_nil;
    std::string errmsg;
    std::vector<result::result_pair> res_list;
    do
    {
      MYSQL_RES* my_res = mysql_store_result(mysql_);
      if (my_res == NULL && mysql_field_count(mysql_) != 0)
      {
        errn = make_errno(mysql_errno(mysql_));
        errmsg = mysql_error(mysql_);
        break;
      }

      res_list.push_back(std::make_pair(my_res, (size_t)mysql_affected_rows(mysql_)));

      /// more results? -1 = no, >0 = error, 0 = yes (keep looping)
      if ((status = mysql_next_result(mysql_)) > 0)
      {
        errn = make_errno(mysql_errno(mysql_));
        errmsg = mysql_error(mysql_);
        break;
      }
    }
    while (status == 0);

    if (errn != errno_nil)
    {
      BOOST_FOREACH(result::result_pair pr, res_list)
      {
        mysql_free_result(pr.first);
      }
    }
    else if (!res_list.empty())
    {
      if (!res)
      {
        res = boost::make_shared<result>();
      }
      res->res_list_ = res_list;
    }

    return std::make_pair(errn, errmsg);
  }

  void disconnect()
  {
    mysql_close(mysql_);
    mysql_ = NULL;
  }

  strand_t snd_;
  std::string host_;
  uint16_t port_;
  std::string usr_;
  std::string pwd_;
  std::string db_;
  connopt_t opt_;
  MYSQL* mysql_;

  int ref_;
  gce::detail::index_table<boost::shared_ptr<fmt::MemoryWriter> > query_buffer_list_;
};
} /// namespace detail
} /// namespace mysql
} /// namespace gce

#endif /// GCE_MYSQL_DETAIL_CONN_IMPL_HPP
