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
#include <gce/log/record.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <ctime>

namespace gce
{
namespace log
{
class asio_logger
{
typedef boost::asio::io_service io_service_t;
typedef boost::chrono::system_clock system_clock_t;
typedef system_clock_t::time_point time_point_t;
typedef system_clock_t::duration duration_t;
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
    boost::string_ref str = rec.get_log_string();
    level lv = rec.get_level();

    char* buf = buf_.data();
    std::time_t t = system_clock_t::to_time_t(nw);
    std::tm* tm_nw = std::localtime(&t);
    std::strftime(buf, buf_.size(), "%Y-%m-%d %X.", tm_nw);
    duration_t::rep ms = nw.time_since_epoch().count();

    boost::string_ref file;
    int line;

    ms /= ms / t / 1000;
    ms = ms - (t / 1000 * (ms / t * 1000));
    std::cout << "[" << buf << ms << "] [" << to_string(lv) << 
      "] ";

    if (!tag.empty())
    {
      std::cout << "[" << tag << "] ";
    }

    if (rec.get_meta(file) && !file.empty())
    {
      std::cout << "[" << file << "] ";
    }

    if (rec.get_meta(line))
    {
      std::cout << "[line: " << line << "] ";
    }

    std::cout << str << std::endl;
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
