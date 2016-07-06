///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_HTTP_REQUEST_HPP
#define GCE_HTTP_REQUEST_HPP

#include <gce/http/config.hpp>
#include <gce/http/header.hpp>
#include <boost/shared_ptr.hpp>

namespace gce
{
namespace http
{
static match_t const as_request = atom("as_request");

/// Http request from client.
struct request
{
  request()
    : http_major_(0)
    , http_minor_(0)
    , upgrade_(false)
  {
  }

  std::string errmsg_; /// If not empty means http parser error.
  std::string method_;
  std::string uri_;
  int http_major_;
  int http_minor_;
  std::vector<header> headers_;
  std::string body_;
  bool upgrade_; /// If true then all data in body_(e.g. websocket).
};

typedef boost::shared_ptr<request> request_ptr;
} /// namespace http
} /// namespace gce

#endif /// GCE_HTTP_REQUEST_HPP
