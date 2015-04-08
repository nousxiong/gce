// (C) Copyright Ning Ding 2013.8
// lordoffox@gmail.com
// Distributed under the Boost Software License, Version 1.0. (See accompany-
// ing file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADATA_CPP2LUA_HPP_HEADER_
#define ADATA_CPP2LUA_HPP_HEADER_

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "adata.hpp"
#include "adata_corec.hpp"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void push(lua_State * L, uint8_t v)
    {
      lua_pushinteger(L, v);
    }

    ADATA_INLINE void push(lua_State * L, int8_t v)
    {
      lua_pushinteger(L, v);
    }

    ADATA_INLINE void push(lua_State * L, uint16_t v)
    {
      lua_pushinteger(L, v);
    }

    ADATA_INLINE void push(lua_State * L, int16_t v)
    {
      lua_pushinteger(L, v);
    }

    ADATA_INLINE void push(lua_State * L, uint32_t v)
    {
      lua_pushinteger(L, v);
    }

    ADATA_INLINE void push(lua_State * L, int32_t v)
    {
      lua_pushinteger(L, v);
    }

    ADATA_INLINE void push(lua_State * L, int64_t v)
    {
      lua::lua_pushint64(L, v);
    }

    ADATA_INLINE void push(lua_State * L, uint64_t v)
    {
      lua::lua_pushuint64(L, v);
    }


    ADATA_INLINE void push(lua_State * L, float v)
    {
      lua_pushnumber(L, (lua_Number)v);
    }

    ADATA_INLINE void push(lua_State * L, double v)
    {
      lua_pushnumber(L, v);
    }

    template<typename alloc>
    ADATA_INLINE void push(lua_State * L, const ::std::basic_string<char, ::std::char_traits<char>, alloc>& v)
    {
      lua_pushlstring(L, v.data(), v.length());
    }

    ADATA_INLINE void load(lua_State * L, int8_t& v)
    {
      v = (int8_t)lua_tointeger(L, -1);
    }

    ADATA_INLINE void load(lua_State * L, uint8_t& v)
    {
      v = (uint8_t)lua_tointeger(L, -1);
    }

    ADATA_INLINE void load(lua_State * L, int16_t& v)
    {
      v = (int16_t)lua_tointeger(L, -1);
    }

    ADATA_INLINE void load(lua_State * L, uint16_t& v)
    {
      v = (uint16_t)lua_tointeger(L, -1);
    }

    ADATA_INLINE void load(lua_State * L, int32_t& v)
    {
      v = (int32_t)lua_tointeger(L, -1);
    }

    ADATA_INLINE void load(lua_State * L, uint32_t& v)
    {
      v = (uint32_t)lua_tointeger(L, -1);
    }

    ADATA_INLINE void load(lua_State * L, int64_t& v)
    {
      number64_type it = { et_int64_unknow };
      lua_tonumber64(L, -1, &it);
      switch (it.type)
      {
      case et_int64_lua_number:
      {
        v = (int64_t)it.value.d64;
        break;
      }
      case et_int64_int64:
      {
        v = (int64_t)it.value.i64;
        break;
      }
      case et_int64_uint64:
      {
        v = (int64_t)it.value.u64;
        break;
      }
      default:
      {
        v = 0;
      }
      }
    }

    ADATA_INLINE void load(lua_State * L, uint64_t& v)
    {
      lua::number64_type it = { et_int64_unknow };
      lua::lua_tonumber64(L, -1, &it);
      switch (it.type)
      {
      case et_int64_lua_number:
      {
        v = (int64_t)it.value.d64;
        break;
      }
      case et_int64_int64:
      {
        v = (int64_t)it.value.i64;
        break;
      }
      case et_int64_uint64:
      {
        v = (int64_t)it.value.u64;
        break;
      }
      default:
      {
        v = 0;
      }
      }
    }

    ADATA_INLINE void load(lua_State * L, float& v)
    {
      v = (float)lua_tonumber(L, -1);
    }

    ADATA_INLINE void load(lua_State * L, double v)
    {
      v = lua_tonumber(L, -1);
    }

    template<typename alloc>
    ADATA_INLINE void load(lua_State * L, ::std::basic_string<char, ::std::char_traits<char>, alloc>& v)
    {
      size_t l = 0;
      const char * s = lua_tolstring(L, -1, &l);
      v.assign(s, l);
    }

    ADATA_INLINE size_t seq_len(lua_State * L, int idx)
    {
#if LUA_VERSION_NUM == 501
      return lua_objlen(L, idx);
#else
      return lua_rawlen(L, idx);
#endif
    }

  }
}

#endif /// ADATA_CPP2LUA_HPP_HEADER_

