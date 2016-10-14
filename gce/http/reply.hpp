///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_HTTP_REPLY_HPP
#define GCE_HTTP_REPLY_HPP

#include <gce/http/config.hpp>
#include <gce/http/header.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace gce
{
namespace http
{
/// A reply to be sent to a client.
struct reply
{
  /// The status of the reply.
  enum status
  {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503
  } stat_;

  reply()
    : stat_(status::ok)
    , http_major_(1)
    , http_minor_(0)
    , upgrade_(false)
  {
  }

  explicit reply(status stat)
    : stat_(stat)
    , http_major_(1)
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
    stat_ = status::ok;
    errmsg_.clear();
    http_major_ = 1;
    http_minor_ = 0;
    headers_.clear();
    content_.clear();
    upgrade_ = false;
    version_.clear();
  }

  /// If not empty means http parser error.
  std::string errmsg_;

  /// Http version.
  int http_major_;
  int http_minor_;

  /// The headers to be included in the reply.
  std::vector<header> headers_;

  /// The content to be sent in the reply.
  fmt::MemoryWriter content_;
  /// If true then all data in content_(e.g. websocket).
  bool upgrade_;

  /// Internal use.
  std::string version_;
};

typedef boost::shared_ptr<reply> reply_ptr;

static void write(reply_ptr rep, fmt::CStringRef format_str, fmt::ArgList args)
{
  rep->content_.write(format_str, args);
}

FMT_VARIADIC(void, write, reply_ptr, fmt::CStringRef)

static void write(reply_ptr rep, char const* buf, size_t len)
{
  rep->content_ << fmt::StringRef(buf, len);
}

static void write(reply_ptr rep, fmt::MemoryWriter& content)
{
  rep->content_ << fmt::StringRef(content.data(), content.size());
}

namespace status_strings
{
const std::string ok =
  "200 OK\r\n";
const std::string created =
  "201 Created\r\n";
const std::string accepted =
  "202 Accepted\r\n";
const std::string no_content =
  "204 No Content\r\n";
const std::string multiple_choices =
  "300 Multiple Choices\r\n";
const std::string moved_permanently =
  "301 Moved Permanently\r\n";
const std::string moved_temporarily =
  "302 Moved Temporarily\r\n";
const std::string not_modified =
  "304 Not Modified\r\n";
const std::string bad_request =
  "400 Bad Request\r\n";
const std::string unauthorized =
  "401 Unauthorized\r\n";
const std::string forbidden =
  "403 Forbidden\r\n";
const std::string not_found =
  "404 Not Found\r\n";
const std::string internal_server_error =
  "500 Internal Server Error\r\n";
const std::string not_implemented =
  "501 Not Implemented\r\n";
const std::string bad_gateway =
  "502 Bad Gateway\r\n";
const std::string service_unavailable =
  "503 Service Unavailable\r\n";

static boost::asio::const_buffer to_buffer(reply::status stat)
{
  switch (stat)
  {
  case reply::ok:
    return boost::asio::buffer(ok);
  case reply::created:
    return boost::asio::buffer(created);
  case reply::accepted:
    return boost::asio::buffer(accepted);
  case reply::no_content:
    return boost::asio::buffer(no_content);
  case reply::multiple_choices:
    return boost::asio::buffer(multiple_choices);
  case reply::moved_permanently:
    return boost::asio::buffer(moved_permanently);
  case reply::moved_temporarily:
    return boost::asio::buffer(moved_temporarily);
  case reply::not_modified:
    return boost::asio::buffer(not_modified);
  case reply::bad_request:
    return boost::asio::buffer(bad_request);
  case reply::unauthorized:
    return boost::asio::buffer(unauthorized);
  case reply::forbidden:
    return boost::asio::buffer(forbidden);
  case reply::not_found:
    return boost::asio::buffer(not_found);
  case reply::internal_server_error:
    return boost::asio::buffer(internal_server_error);
  case reply::not_implemented:
    return boost::asio::buffer(not_implemented);
  case reply::bad_gateway:
    return boost::asio::buffer(bad_gateway);
  case reply::service_unavailable:
    return boost::asio::buffer(service_unavailable);
  default:
    return boost::asio::buffer(internal_server_error);
  }
}

} /// namespace status_strings
} /// namespace http
} /// namespace gce

#endif /// GCE_HTTP_REPLY_HPP
