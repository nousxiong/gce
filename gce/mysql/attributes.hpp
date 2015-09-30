///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_ATTRIBUTES_HPP
#define GCE_MYSQL_ATTRIBUTES_HPP

#include <gce/mysql/config.hpp>

namespace gce
{
namespace mysql
{
struct attributes
{
  attributes()
    : thread_num_(boost::thread::hardware_concurrency())
  {
  }

  size_t thread_num_;
  std::vector<thread_callback_t> thread_begin_cb_list_;
  std::vector<thread_callback_t> thread_end_cb_list_;
  log::logger_t lg_;
};
}
}

#endif /// GCE_MYSQL_ATTRIBUTES_HPP
