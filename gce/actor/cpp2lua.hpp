///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ACTOR_CPP2LUA_HPP
#define GCE_ACTOR_CPP2LUA_HPP

#include <gce/actor/config.hpp>
#include <gce/actor/detail/lua_wrap.hpp>

namespace gce
{
namespace lua
{
///------------------------------------------------------------------------------
/// service_id
///------------------------------------------------------------------------------
inline void load(lua_State* L, int arg, gce::svcid_t& o)
{
  detail::lua::load(L, arg, o);
}

inline void push(lua_State* L, gce::svcid_t const& o)
{
  detail::lua::push(L, o);
}
///------------------------------------------------------------------------------
/// actor_id
///------------------------------------------------------------------------------
inline void load(lua_State* L, int arg, gce::aid_t& o)
{
  detail::lua::load(L, arg, o);
}

inline void push(lua_State* L, gce::aid_t const& o)
{
  detail::lua::push(L, o);
}
///------------------------------------------------------------------------------
/// duration
///------------------------------------------------------------------------------
inline void load(lua_State* L, int arg, gce::duration_t& o)
{
  detail::lua::load(L, arg, o);
}

inline void push(lua_State* L, gce::duration_t const& o)
{
  detail::lua::push(L, o);
}
///------------------------------------------------------------------------------
/// match
///------------------------------------------------------------------------------
inline void load(lua_State* L, int arg, gce::match_t& o)
{
  detail::lua::load(L, arg, o);
}

inline void push(lua_State* L, gce::match_t const& o)
{
  detail::lua::push(L, o);
}
///------------------------------------------------------------------------------
/// netopt
///------------------------------------------------------------------------------
inline void load(lua_State* L, int arg, gce::netopt_t& o)
{
  detail::lua::load(L, arg, o);
}

inline void push(lua_State* L, gce::netopt_t const& o)
{
  detail::lua::push(L, o);
}
///------------------------------------------------------------------------------
/// message, errcode, chunk
///------------------------------------------------------------------------------
typedef detail::lua::message message;
typedef detail::lua::errcode errcode;
typedef detail::lua::chunk chunk;

/// helper function
template <typename Tag>
typename Tag::object_t* from_lua(lua_State* L, int arg)
{
  return detail::lua::to_obj<Tag>(L, arg);
}

template <typename Object>
Object* from_lua(lua_State* L, int arg, char const* name)
{
  return detail::lua::to_obj<Object>(L, arg, name);
}
///------------------------------------------------------------------------------
}
}

#endif /// GCE_ACTOR_CPP2LUA_HPP
