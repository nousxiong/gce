///
/// Copyright (c) 2009-2014 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_LUA_OBJECT_HPP
#define GCE_ACTOR_LUA_OBJECT_HPP

#include <gce/actor/config.hpp>
#include <gce/lualib/all.hpp>
#include <cstring>

namespace gce
{
class message;
namespace lua
{
enum
{
  /// both in adata and amsg means userdata
  ty_pattern = 0,
  ty_message,
  ty_response,
  ty_chunk,
  ty_errcode,

  /// in adata means adtype, in amsg means userdata
  ty_match,
  ty_duration,
  ty_actor_id,
  ty_service_id,
  ty_userdef,

  /// lua basic types
  ty_lua,

  /// none gce userdata
  ty_other,

  ty_num
};

class basic_object
{
public:
  basic_object() {}
  virtual ~basic_object() {}

public:
  virtual void pack(gce::message&) = 0;
  virtual void unpack(gce::message&) = 0;
  virtual int gcety() const = 0;
};
}
}

#endif /// GCE_ACTOR_LUA_OBJECT_HPP
