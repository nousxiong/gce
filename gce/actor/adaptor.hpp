///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_ADAPTOR_HPP
#define GCE_ACTOR_ADAPTOR_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/actor.hpp>
#include <boost/asio.hpp>

namespace gce
{
class adaptor
{
public:
  adaptor(stackless_actor a, errcode_t& ec, size_t& bytes_transferred)
    : a_(a)
    , ec_(ec)
    , bytes_transferred_(&bytes_transferred)
    , itr_(0)
    , signal_number_(0)
  {
  }

  adaptor(stackless_actor a, errcode_t& ec, boost::asio::ip::tcp::resolver::iterator& itr)
    : a_(a)
    , ec_(ec)
    , bytes_transferred_(0)
    , itr_(&itr)
    , signal_number_(0)
  {
  }

  adaptor(stackless_actor a, errcode_t& ec)
    : a_(a)
    , ec_(ec)
    , bytes_transferred_(0)
    , itr_(0)
    , signal_number_(0)
  {
  }

  adaptor(stackless_actor a, errcode_t& ec, int& signal_number)
    : a_(a)
    , ec_(ec)
    , bytes_transferred_(0)
    , itr_(0)
    , signal_number_(&signal_number)
  {
  }

public:
  void operator()(errcode_t const& ec, size_t bytes_transferred)
  {
    GCE_ASSERT(bytes_transferred_ != 0)(bytes_transferred);
    ec_ = ec;
    *bytes_transferred_ = bytes_transferred;
    a_.sync_resume();
  }

  void operator()(errcode_t const& ec, boost::asio::ip::tcp::resolver::iterator itr)
  {
    GCE_ASSERT(itr_ != 0);
    ec_ = ec;
    *itr_ = itr;
    a_.sync_resume();
  }

  void operator()(errcode_t const& ec)
  {
    ec_ = ec;
    a_.sync_resume();
  }

  void operator()(errcode_t const& ec, int signal_number)
  {
    GCE_ASSERT(signal_number_ != 0);
    ec_ = ec;
    *signal_number_ = signal_number;
    a_.sync_resume();
  }

private:
  stackless_actor a_;
  errcode_t& ec_;
  size_t* bytes_transferred_;
  boost::asio::ip::tcp::resolver::iterator* itr_;
  int* signal_number_;
};
}

#endif /// GCE_ACTOR_ADAPTOR_HPP
