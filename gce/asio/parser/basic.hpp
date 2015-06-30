///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_PARSER_BASIC_HPP
#define GCE_ASIO_PARSER_BASIC_HPP

#include <gce/asio/config.hpp>

namespace gce
{
namespace asio
{
namespace parser
{
/// using length to parse msg
struct length
{
  struct header
  {
    bool finish_;
    size_t header_size_;
    size_t body_size_;
  };

  /// invoke when recv header, may call multi, until header parsing complete
  virtual header on_recv_header(boost::asio::const_buffer, message&) = 0;

  /// invoke before send msg
  virtual void on_send(message&, message&) = 0;

  /// get msg max header size, must be reentrant
  virtual size_t max_header_size() const = 0;
};

/// using regex to parse msg
struct regex
{
  /// invoke after finish recv a message
  virtual void on_recv(message&) = 0;

  /// get current regex expression
  virtual std::string get_expr() const = 0;
};
}
}
}

#endif /// GCE_ASIO_PARSER_BASIC_HPP
