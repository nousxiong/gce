
#ifndef GCE_HTTP_DETAIL_REPLY_PARSER_HPP
#define GCE_HTTP_DETAIL_REPLY_PARSER_HPP

#include <gce/http/config.hpp>
#include <gce/http/reply.hpp>
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
class reply_parser
{
  enum status
  {
    null = 0,
    field,
    value,
    upgrade, /// under this status, parser will not parse http.
  };

public:
  /// Construct ready to parse the reply method.
  reply_parser()
    : replies_(0)
    , completed_(false)
    , stat_(null)
  {
    parser_settings_.on_message_begin    = reply_parser::on_message_begin;
    parser_settings_.on_message_complete = reply_parser::on_message_complete;
    parser_settings_.on_headers_complete = reply_parser::on_headers_complete;
    parser_settings_.on_header_field     = reply_parser::header_field_cb;
    parser_settings_.on_header_value     = reply_parser::header_value_cb;
    parser_settings_.on_url              = reply_parser::url_cb;
    parser_settings_.on_status           = reply_parser::status_cb;
    parser_settings_.on_chunk_header     = reply_parser::on_chunk_header;
    parser_settings_.on_chunk_complete   = reply_parser::on_chunk_complete;
    parser_settings_.on_body             = reply_parser::body_cb;
    
    http_parser_init(&parser_, HTTP_RESPONSE);
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

  /// Parse some data. The tribool return value is true when a complete reply
  /// has been parsed, false if the data is invalid, indeterminate when more
  /// data is required. 
  boost::tribool parse(std::deque<reply_ptr>& replies, char const* buf, size_t len)
  {
    if (is_upgrade())
    {
      if (replies_->empty())
      {
        replies_->push_back(boost::make_shared<reply>());
      }
      reply_ptr rep = replies_->back();
      rep->upgrade_ = true;
      write(rep, buf, len);
      return true;
    }

    replies_ = &replies;
    int nparsed = http_parser_execute(&parser_, &parser_settings_, buf, len);
    if (parser_.upgrade)
    {
      reply_ptr last = replies_->back();
      last->upgrade_ = true;

      reply_ptr rep = boost::make_shared<reply>();
      rep->upgrade_ = true;
      write(rep, buf + nparsed, len - nparsed);
      replies_->push_back(rep);

      stat_ = upgrade;
      return true;
    }
    else if (nparsed != len)
    {
      if (!replies_->empty())
      {
        reply_ptr last = replies_->back();
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
    reply_ptr rep = boost::make_shared<reply>();
    replies_->push_back(rep);
    return 0;
  }

  int handle_message_complete()
  {
    completed_ = true;
    return 0;
  }

  int handle_headers_complete()
  {
    reply_ptr rep = replies_->back();
    rep->upgrade_ = parser_.upgrade ? true : false;
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
    return 0;
  }

  int handle_status(char const* buf, size_t len)
  {
    reply_ptr rep = replies_->back();
    std::string status(buf, len);
    return 0;
  }

  int handle_header_field(char const* buf, size_t len)
  {
    reply_ptr rep = replies_->back();
    if (stat_ == null || stat_ == value)
    {
      rep->headers_.push_back(header());
    }

    header& hdr = rep->headers_.back();
    hdr.name_.append(buf, len);
    stat_ = field;
    return 0;
  }

  int handle_header_value(char const* buf, size_t len)
  {
    reply_ptr rep = replies_->back();
    assert(!rep->headers_.empty());
    header& hdr = rep->headers_.back();
    hdr.value_.append(buf, len);
    stat_ = value;
    return 0;
  }

  int handle_body(char const* buf, size_t len)
  {
    reply_ptr rep = replies_->back();
    write(rep, buf, len);
    return 0;
  }

private:
  static int on_message_begin(http_parser* parser)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_message_begin();
  }

  static int on_message_complete(http_parser* parser)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_message_complete();
  }
  
  static int on_headers_complete(http_parser* parser)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_headers_complete();
  }
  
  static int on_chunk_header(http_parser* parser)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_chunk_header();
  }
  
  static int on_chunk_complete(http_parser* parser)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_chunk_complete();
  }

  static int url_cb(http_parser* parser, char const* buf, size_t len)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_url(buf, len);
  }

  static int status_cb(http_parser* parser, char const* buf, size_t len)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_status(buf, len);
  }

  static int header_field_cb(http_parser* parser, char const* buf, size_t len)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_header_field(buf, len);
  }

  static int header_value_cb(http_parser* parser, char const* buf, size_t len)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_header_value(buf, len);
  }

  static int body_cb(http_parser *parser, char const* buf, size_t len)
  {
    reply_parser* self = (reply_parser*)parser->data;
    return self->handle_body(buf, len);
  }

private:
  http_parser_settings parser_settings_;
  http_parser parser_;
  std::deque<reply_ptr>* replies_;
  bool completed_;
  status stat_;
};
} /// namespace detail
} /// namespace http
} /// namespace gce

#endif /// GCE_HTTP_DETAIL_REPLY_PARSER_HPP
