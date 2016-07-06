
#ifndef GCE_HTTP_DETAIL_REQUEST_PARSER_HPP
#define GCE_HTTP_DETAIL_REQUEST_PARSER_HPP

#include <gce/http/config.hpp>
#include <gce/http/request.hpp>
#include <boost/make_shared.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <deque>
#include <cassert>

namespace gce
{
namespace http
{
namespace detail
{
class request_parser
{
  enum status
  {
    null = 0,
    field,
    value,
    upgrade, /// under this status, parser will not parse http.
  };

public:
  /// Construct ready to parse the request method.
  request_parser()
    : requests_(0)
    , completed_(false)
    , stat_(null)
  {
    parser_settings_.on_message_begin    = request_parser::on_message_begin;
    parser_settings_.on_message_complete = request_parser::on_message_complete;
    parser_settings_.on_headers_complete = request_parser::on_headers_complete;
    parser_settings_.on_header_field     = request_parser::header_field_cb;
    parser_settings_.on_header_value     = request_parser::header_value_cb;
    parser_settings_.on_url              = request_parser::url_cb;
    parser_settings_.on_status           = request_parser::status_cb;
    parser_settings_.on_chunk_header     = request_parser::on_chunk_header;
    parser_settings_.on_chunk_complete   = request_parser::on_chunk_complete;
    parser_settings_.on_body             = request_parser::body_cb;
    
    http_parser_init(&parser_, HTTP_REQUEST);
    reset();
  }

  /// Reset to initial parser state.
  void reset()
  {
    completed_ = false;
    stat_ = null;
    parser_.data = this;
  }

  bool is_upgrade() const
  {
    return stat_ == upgrade;
  }

  /// Parse some data. The tribool return value is true when a complete request
  /// has been parsed, false if the data is invalid, indeterminate when more
  /// data is required. 
  boost::tribool parse(std::deque<request_ptr>& requests, char const* buf, size_t len)
  {
    if (is_upgrade())
    {
      if (requests_->empty())
      {
        requests_->push_back(boost::make_shared<request>());
      }
      request_ptr req = requests_->back();
      req->upgrade_ = true;
      req->body_.append(buf, len);
      return true;
    }

    requests_ = &requests;
    int nparsed = http_parser_execute(&parser_, &parser_settings_, buf, len);
    if (parser_.upgrade)
    {
      request_ptr last = requests_->back();
      last->upgrade_ = true;

      request_ptr req = boost::make_shared<request>();
      req->upgrade_ = true;
      req->body_.append(buf + nparsed, len - nparsed);
      requests_->push_back(req);

      stat_ = upgrade;
      return true;
    }
    else if (nparsed != len)
    {
      if (!requests_->empty())
      {
        request_ptr last = requests_->back();
        last->errmsg_ = http_errno_description((http_errno)parser_.http_errno);
      }
      return false;
    }

    if (completed_)
    {
      return true;
    }
    else
    {
      return boost::indeterminate;
    }
  }

private:
  int handle_message_begin()
  {
    reset();
    request_ptr req = boost::make_shared<request>();
    requests_->push_back(req);
    return 0;
  }

  int handle_message_complete()
  {
    completed_ = true;
    return 0;
  }

  int handle_headers_complete()
  {
    request_ptr req = requests_->back();
    req->http_major_ = parser_.http_major;
    req->http_minor_ = parser_.http_minor;
    req->upgrade_ = parser_.upgrade ? true : false;
    return 0;
  }

  int handle_chunk_header()
  {
    return 0;
  }

  int handle_chunk_complete()
  {
    return 0;
  }

  int handle_url(char const* buf, size_t len)
  {
    request_ptr req = requests_->back();
    req->method_ = (char const*)http_method_str((http_method)parser_.method);
    req->uri_.append(buf, len);
    return 0;
  }

  int handle_status(char const* buf, size_t len)
  {
    std::string status(buf, len);
    return 0;
  }

  int handle_header_field(char const* buf, size_t len)
  {
    request_ptr req = requests_->back();
    if (stat_ == null || stat_ == value)
    {
      req->headers_.push_back(header());
    }

    header& hdr = req->headers_.back();
    hdr.name_.append(buf, len);
    stat_ = field;
    return 0;
  }

  int handle_header_value(char const* buf, size_t len)
  {
    request_ptr req = requests_->back();
    assert(!req->headers_.empty());
    header& hdr = req->headers_.back();
    hdr.value_.append(buf, len);
    stat_ = value;
    return 0;
  }

  int handle_body(char const* buf, size_t len)
  {
    request_ptr req = requests_->back();
    req->body_.append(buf, len);
    return 0;
  }

private:
  static int on_message_begin(http_parser* parser)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_message_begin();
  }

  static int on_message_complete(http_parser* parser)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_message_complete();
  }
  
  static int on_headers_complete(http_parser* parser)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_headers_complete();
  }
  
  static int on_chunk_header(http_parser* parser)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_chunk_header();
  }
  
  static int on_chunk_complete(http_parser* parser)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_chunk_complete();
  }

  static int url_cb(http_parser* parser, char const* buf, size_t len)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_url(buf, len);
  }

  static int status_cb(http_parser* parser, char const* buf, size_t len)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_status(buf, len);
  }

  static int header_field_cb(http_parser* parser, char const* buf, size_t len)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_header_field(buf, len);
  }

  static int header_value_cb(http_parser* parser, char const* buf, size_t len)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_header_value(buf, len);
  }

  static int body_cb(http_parser *parser, char const* buf, size_t len)
  {
    request_parser* self = (request_parser*)parser->data;
    return self->handle_body(buf, len);
  }

private:
  http_parser_settings parser_settings_;
  http_parser parser_;
  std::deque<request_ptr>* requests_;
  bool completed_;
  status stat_;
};
} /// namespace detail
} /// namespace http
} /// namespace gce

#endif /// GCE_HTTP_DETAIL_REQUEST_PARSER_HPP
