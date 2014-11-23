///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_LOG_SCOPE_HPP
#define GCE_LOG_SCOPE_HPP

#include <gce/log/config.hpp>
#include <gce/log/record.hpp>
#include <vector>

namespace gce
{
namespace log
{
class scope
{
  friend class record;

public:
  explicit scope(logger_t& lg, std::size_t reserve = 1)
    : lg_(lg)
  {
    record_list_.reserve(reserve);
  }

  ~scope()
  {
    lg_(record_list_);
  }

private:
  logger_t& get_logger()
  {
    return lg_;
  }

  record& add_record(level lv)
  {
    record_list_.push_back(record(lg_, lv, system_clock_t::now()));
    return record_list_.back();
  }

private:
  logger_t& lg_;
  std::vector<record> record_list_;
};
}
}

#endif /// GCE_LOG_SCOPE_HPP
