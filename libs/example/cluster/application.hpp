///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_EXAMPLE_CLUSTER_APPLICATION_HPP
#define GCE_EXAMPLE_CLUSTER_APPLICATION_HPP

#include "config.hpp"

namespace usr
{
class application
{
public:
  application(char const* ctxid, gce::log::logger_t& lg);
  ~application();

public:
  void run(char const* script);

private:
  static gce::context::init_t make_init(char const* ctxid, gce::log::logger_t& lg);

private:
  std::string const ctxid_;
  gce::log::logger_t& lg_;
  gce::context ctx_;
};
}

#endif /// GCE_EXAMPLE_CLUSTER_APPLICATION_HPP
