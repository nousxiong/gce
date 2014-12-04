///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_LOG_DETAIL_LOGGER_HPP
#define GCE_LOG_DETAIL_LOGGER_HPP

#include <gce/log/config.hpp>
#include <gce/log/record.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/array.hpp>
#include <iostream>
#include <ctime>

namespace gce
{
namespace log
{
namespace detail
{
inline void output_impl(
  gce::log::record& rec, 
  std::string const& tag, 
  gce::log::time_point_t nw, 
  boost::array<char, 80>& buffer
  )
{
  boost::string_ref str = rec.get_log_string();
  level lv = rec.get_level();

  char* buf = buffer.data();
  std::time_t t = system_clock_t::to_time_t(nw);
  std::tm* tm_nw = std::localtime(&t);
  std::strftime(buf, buffer.size(), "%Y-%m-%d %X.", tm_nw);
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
}
}
}

#endif /// GCE_LOG_DETAIL_LOGGER_HPP
