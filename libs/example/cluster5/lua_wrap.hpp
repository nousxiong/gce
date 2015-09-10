///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef CLUSTER5_LUA_WRAP_HPP
#define CLUSTER5_LUA_WRAP_HPP

#include "string_hash.hpp"
#include <gce/actor/all.hpp>
#include <gce/lualib/all.hpp>

namespace cluster5
{
namespace detail
{
namespace lua
{
inline int hash(lua_State* L)
{
  char const* str = luaL_checkstring(L, 1);
  uint64_t h = string_hash(str);
  gce::lua::push(L, gce::make_match(h));
  return 1;
}
}
}
}

#endif /// CLUSTER5_LUA_WRAP_HPP
