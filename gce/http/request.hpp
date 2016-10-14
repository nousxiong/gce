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
/// Http request from client.
struct request
{
  request()
    : http_major_(1)
    , http_minor_(0)
    , upgrade_(false)
  {
  }

  boost::string_ref get_content() const
  {
    return boost::string_ref(content_.data(), content_.size());
  }

  void add_header(char const* name, std::string const& value)
  {
    headers_.push_back(header());
    header& h = headers_.back();
    h.name_ = name;
    h.value_ = value;
  }

  void clear()
  {
    errmsg_.clear();
    method_.clear();
    uri_.clear();
    http_major_ = 1;
    http_minor_ = 0;
    headers_.clear();
    content_.clear();
    upgrade_ = false;
    version_.clear();
  }

  /// If not empty means http parser error.
  std::string errmsg_;

  std::string method_;
  std::string uri_;
  int http_major_;
  int http_minor_;
  std::vector<header> headers_;
  fmt::MemoryWriter content_;

  /// If true then all data in body_(e.g. websocket).
  bool upgrade_;

  /// Internal use.
  std::string version_;
};

typedef boost::shared_ptr<request> request_ptr;

static void write(request_ptr req, fmt::CStringRef format_str, fmt::ArgList args)
{
  req->content_.write(format_str, args);
}

FMT_VARIADIC(void, write, request_ptr, fmt::CStringRef)

static void write(request_ptr req, char const* buf, size_t len)
{
  req->content_ << fmt::StringRef(buf, len);
}

static void write(request_ptr req, fmt::MemoryWriter& content)
{
  req->content_ << fmt::StringRef(content.data(), content.size());
}
} /// namespace http
} /// namespace gce

#endif /// GCE_HTTP_REQUEST_HPP
