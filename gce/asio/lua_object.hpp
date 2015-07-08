///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_LUA_OBJECT_HPP
#define GCE_ASIO_LUA_OBJECT_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/parser/basic.hpp>
#include <gce/lualib/all.hpp>
#include <cstring>

namespace gce
{
class message;
namespace asio
{
namespace lua
{
enum
{
  ty_ssl_context = gce::lua::ty_num,
  ty_ssl_stream_impl,

  ty_tcp_endpoint,
  ty_tcp_endpoint_itr,
  ty_tcp_socket_impl,

  ty_num
};

enum parser_type
{
  plength = 0,
  pregex
};

namespace parser
{
struct base
{
  virtual parser_type get_type() const = 0;
};

struct length
  : public base
{
  virtual boost::shared_ptr<asio::parser::length> get_impl() = 0;
  parser_type get_type() const { return plength; }
};

struct regex
  : public base
{
  virtual boost::shared_ptr<asio::parser::regex> get_impl() = 0;
  parser_type get_type() const { return pregex; }
};
} /// namespace parser
} /// namespace lua
} /// namespace asio
} /// namespace gce

#endif /// GCE_ASIO_LUA_OBJECT_HPP
