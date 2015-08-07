///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_DETAIL_YIELDER_HPP
#define GCE_ACTOR_DETAIL_YIELDER_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/error_code.hpp>
#include <gce/actor/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace gce
{
namespace detail
{
struct yielder
{
  struct basic_host
  {
    virtual void yield() = 0;
    virtual void resume() = 0;
  };

  typedef boost::asio::ip::tcp::resolver::iterator resolver_iterator;

  explicit yielder(basic_host& host)
    : host_(&host)
    , ec_(0)
    , bytes_transferred_(0)
    , itr_(0)
  {
  }

  yielder(basic_host& host, errcode_t& ec)
    : host_(&host)
    , ec_(&ec)
    , bytes_transferred_(0)
    , itr_(0)
  {
  }

  ~yielder()
  {
  }

  void yield()
  {
    host_->yield();
  }

  void operator()(errcode_t const& ec) const
  {
    if (ec_ != 0)
    {
      *ec_ = ec;
    }
    host_->resume();
  }

  void operator()(errcode_t const& ec, size_t bytes_transferred) const
  {
    if (ec_ != 0)
    {
      *ec_ = ec;
    }
    if (bytes_transferred_ != 0)
    {
      *bytes_transferred_ = bytes_transferred;
    }
    host_->resume();
  }

  void operator()(errcode_t const& ec, resolver_iterator itr) const
  {
    if (ec_ != 0)
    {
      *ec_ = ec;
    }
    if (itr_ != 0)
    {
      *itr_ = itr;
    }
    host_->resume();
  }

  yielder& operator[](errcode_t& ec)
  {
    ec_ = &ec;
    return *this;
  }

  yielder& operator[](size_t& bytes_transferred)
  {
    bytes_transferred_ = &bytes_transferred;
    return *this;
  }

  yielder& operator[](resolver_iterator& itr)
  {
    itr_ = &itr;
    return *this;
  }

  basic_host* host_;
  errcode_t* ec_;
  size_t* bytes_transferred_;
  resolver_iterator* itr_;
};
}
}

#endif /// GCE_ACTOR_DETAIL_YIELDER_HPP
