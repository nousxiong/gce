///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_DETAIL_SCOPE_HPP
#define GCE_DETAIL_SCOPE_HPP

#include <gce/config.hpp>
#include <boost/function.hpp>

namespace gce
{
namespace detail
{
class scope
{
  scope(scope const&);
  scope& operator=(scope const&);

  void operator==(scope const&) const;
  void operator!=(scope const&) const;

public:
  typedef boost::function<void ()> cb_t;

public:
  scope()
  {
  }

  template <typename F>
  explicit scope(F const& f)
    : cb_(f)
  {
  }

  template <typename F, typename A>
  explicit scope(F const& f, A const& a)
    : cb_(f, a)
  {
  }

  ~scope()
  {
    if (cb_)
    {
      cb_();
    }
  }

public:
  void reset()
  {
    cb_.clear();
  }

  void reset(cb_t const& cb)
  {
    cb_ = cb;
  }

  operator bool() const
  {
    return cb_ != 0;
  }

  bool operator!() const
  {
    return cb_ == 0;
  }

private:
  cb_t cb_;
};
///----------------------------------------------------------------------------
/// scope_handler
///----------------------------------------------------------------------------
template <typename Handler>
class scope_handler
{
public:
  explicit scope_handler(Handler const& h)
    : h_(h)
    , off_(false)
  {
  }

  ~scope_handler()
  {
    if (!off_)
    {
      h_();
    }
  }

public:
  void reset()
  {
    off_ = true;
  }

  void reset(Handler const& h)
  {
    h_ = h;
    off_ = false;
  }

private:
  Handler h_;
  bool off_;
};
}
}

#endif /// GCE_DETAIL_SCOPE_HPP
