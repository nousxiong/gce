///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_LOG_STD_LOGGER_HPP
#define GCE_LOG_STD_LOGGER_HPP

#include <gce/log/config.hpp>
#include <gce/log/detail/logger.hpp>
#include <boost/thread/locks.hpp> 
#include <boost/thread/mutex.hpp>
#include <boost/thread/null_mutex.hpp>

namespace gce
{
namespace log
{
template <typename Mutex = boost::mutex>
class std_logger
{
  typedef Mutex mutex_t;
public:
  std_logger()
  {
  }

  ~std_logger()
  {
  }

public:
  void output(record& rec, std::string const& tag)
  {
    boost::unique_lock<mutex_t> lock(mtx_);
    time_point_t nw = system_clock_t::now();
    detail::output_impl(rec, tag, nw, buf_);
  }

private:
  mutex_t mtx_;
  boost::array<char, 80> buf_;
};

typedef std_logger<boost::mutex> std_logger_mt;
typedef std_logger<boost::null_mutex> std_logger_st;
}
}

#endif /// GCE_LOG_STD_LOGGER_HPP
