///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_LOG_ASIO_LOGGER_HPP
#define GCE_LOG_ASIO_LOGGER_HPP

#include <gce/log/config.hpp>
#include <gce/log/detail/logger.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>

namespace gce
{
namespace log
{
class asio_logger
{
typedef boost::asio::io_service io_service_t;
public:
  asio_logger()
    : work_(boost::in_place(boost::ref(ios_)))
    , thr_(boost::in_place(boost::bind(&io_service_t::run, &ios_)))
  {
  }

  ~asio_logger()
  {
    work_.reset();
    if (thr_)
    {
      thr_->join();
    }
  }

public:
  void output(record& rec, std::string const& tag)
  {
    time_point_t nw = system_clock_t::now();
    ios_.post(boost::bind(&asio_logger::output_impl, this, rec, tag, nw));
  }

private:
  void output_impl(record& rec, std::string const& tag, time_point_t nw)
  {
    detail::output_impl(rec, tag, nw, buf_);
  }

private:
  io_service_t ios_;
  boost::optional<io_service_t::work> work_;
  boost::optional<boost::thread> thr_;

  boost::array<char, 80> buf_;
};
}
}
#endif /// GCE_LOG_ASIO_LOGGER_HPP
