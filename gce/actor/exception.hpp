///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_EXCEPTION_HPP
#define GCE_ACTOR_EXCEPTION_HPP

#include <gce/actor/config.hpp>
#include <stdexcept>

namespace gce
{
/// GCE base exception
class exception
  : public std::runtime_error
{
public:
  explicit exception(char const* msg)
    : runtime_error(msg)
  {
  }
};

/// exit message base exception
class exit_exception
  : public exception
{
public:
  exit_exception(char const* msg)
    : exception(msg)
  {
  }
};

class normal_exception
  : public exit_exception
{
public:
  normal_exception(char const* msg)
    : exit_exception(msg)
  {
  }
};

class runtime_exception
  : public exit_exception
{
public:
  runtime_exception(char const* msg)
    : exit_exception(msg)
  {
  }
};

class remote_exception
  : public exit_exception
{
public:
  remote_exception(char const* msg)
    : exit_exception(msg)
  {
  }
};

class already_exit_exception
  : public exit_exception
{
public:
  already_exit_exception(char const* msg)
    : exit_exception(msg)
  {
  }
};

class neterr_exception
  : public exit_exception
{
public:
  neterr_exception(char const* msg)
    : exit_exception(msg)
  {
  }
};

/// recv timeout base exception
class timeout_exception
  : public exception
{
public:
  timeout_exception(char const* msg)
    : exception(msg)
  {
  }
};

class recv_timeout_exception
  : public timeout_exception
{
public:
  recv_timeout_exception(char const* msg)
    : timeout_exception(msg)
  {
  }
};

class respond_timeout_exception
  : public timeout_exception
{
public:
  respond_timeout_exception(char const* msg)
    : timeout_exception(msg)
  {
  }
};

/// spawn actor base exception
class spawn_exception
  : public exception
{
public:
  spawn_exception(char const* msg)
    : exception(msg)
  {
  }
};

class no_socket_exception
  : public spawn_exception
{
public:
  no_socket_exception(char const* msg)
    : spawn_exception(msg)
  {
  }
};

class func_not_found_exception
  : public spawn_exception
{
public:
  func_not_found_exception(char const* msg)
    : spawn_exception(msg)
  {
  }
};

/// unsupported protocol exception
class unsupported_protocol_exception
  : public exception
{
public:
  unsupported_protocol_exception(char const* msg)
    : exception(msg)
  {
  }
};

/// script base exception
class script_exception
  : public exception
{
public:
  script_exception(char const* msg)
    : exception(msg)
  {
  }
};

class lua_exception
  : public script_exception
{
public:
  lua_exception(char const* msg)
    : script_exception(msg)
  {
  }
};

}

#endif /// GCE_ACTOR_EXCEPTION_HPP
