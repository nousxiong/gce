// (C) Copyright Ning Ding 2014.8
// lordoffox@gmail.com
// Distributed under the boost Software License, Version 1.0. (See accompany-
// ing file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef l_adata_int64_hpp
#define l_adata_int64_hpp

#ifdef _MSC_VER
# if _MSC_VER <= 1500
#   include "stdint.hpp"
# else
#   include <cstdint>
# endif
#elif __cplusplus < 201103L
# include <stdint.h>
#else
# include <cstdint>
#endif

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include <cmath>

namespace adata {
  namespace lua{
    enum
    {
      et_int64_unknow,
      et_int64_lua_number,
      et_int64_int64,
      et_int64_uint64,
    };

    struct number64_type
    {
      int type;
      int lua_type;
      union
      {
        double    d64;
        int64_t   i64;
        uint64_t  u64;
      }value;
    };

    const int64_t min_lua_integer = -9007199254740992LL;
    const int64_t max_lua_integer = 9007199254740992LL;
    const int64_t max_int64_integer = 9223372036854775807LL;

#if LUA_VERSION_NUM < 503
    static const char * int64_metatable = "int64_meta";
#endif
    static const char * uint64_metatable = "uint64_meta";

    ADATA_INLINE int lua_tonumber64(lua_State * L, int idx, number64_type * it)
    {
      it->lua_type = lua_type(L, idx);
      if (it->lua_type == LUA_TNUMBER)
      {
#if LUA_VERSION_NUM < 503
        lua_Number nv = lua_tonumber(L, idx);
        it->type = et_int64_int64;
        it->value.i64 = (int64_t)nv;
        if ((lua_Number)it->value.i64 != nv)
        {
          it->type = et_int64_lua_number;
          it->value.d64 = lua_tonumber(L, idx);
        }
#else
        if (lua_isinteger(L, idx))
        {
          it->type = et_int64_int64;
          it->value.i64 = lua_tointeger(L, idx);
        }
        else
        {
          it->type = et_int64_lua_number;
          it->value.d64 = lua_tonumber(L, idx);
        }
#endif
      }
      else if (it->lua_type == LUA_TUSERDATA)
      {
        void * ud = lua_touserdata(L, idx);
        if (ud != NULL)
        {
          if (lua_getmetatable(L, idx))
          {
            luaL_getmetatable(L, uint64_metatable);
            if (lua_rawequal(L, -1, -2))
            {
              it->type = et_int64_uint64;
              it->value.u64 = *(uint64_t*)ud;
            }
#if LUA_VERSION_NUM < 503
            else
            {
              lua_pop(L, 1);
              luaL_getmetatable(L, int64_metatable);
              if (lua_rawequal(L, -1, -2))
              {
                it->type = et_int64_int64;
                it->value.i64 = *(int64_t*)ud;
              }
            }
#endif
            lua_pop(L, 2);
          }
        }
      }
      else if (it->lua_type == LUA_TSTRING)
      {
        size_t len = 0;
        const char * str = (const char *)lua_tolstring(L, idx, &len);
        if (len > 0)
        {
          int singed = 0;
          size_t pos = 0;
          if (str[0] == '-')
          {
            ++pos;
            singed = 1;
          }
          uint64_t read_value = 0;
          while (str[pos])
          {
            char cn = str[pos];
            switch (cn)
            {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            {
              read_value *= 10;
              read_value += (cn - '0');
              ++pos;
              break;
            }
            default:
            {
              break;
            }
            }
          }
          size_t remain_len = len - pos;
          const char * endptr = str + pos;
          if (remain_len == 2)
          {
            //try singed int64
            if ((endptr[0] == 'l' && endptr[1] == 'l') || (endptr[0] == 'L' && endptr[1] == 'L'))
            {
              it->type = et_int64_int64;
              it->value.i64 = singed ? -(int64_t)read_value : (int64_t)read_value;
            }
          }
          else if (remain_len == 3)
          {
            //try unsinged int64
            if (!singed)
            {
              if ((endptr[0] == 'u' && endptr[1] == 'l' && endptr[2] == 'l') || (endptr[0] == 'U' && endptr[1] == 'L' && endptr[1] == 'L'))
              {
                it->type = et_int64_uint64;
                it->value.i64 = read_value;
              }
            }
          }
        }
      }
      return 1;
    }

    ADATA_INLINE void lua_pushuint64(lua_State *L, uint64_t n)
    {
      if (n < (1ULL << 53ULL))
      {
        lua_pushnumber(L, (lua_Number)n);
      }
      else
      {
        uint64_t *o = (uint64_t *)lua_newuserdata(L, sizeof(uint64_t));
        *o = n;
#if LUA_VERSION_NUM == 501
        luaL_newmetatable(L, uint64_metatable);
        lua_setmetatable(L, -2);
#else
        luaL_setmetatable(L, uint64_metatable);
#endif
      }
      return;
    }

    ADATA_INLINE void lua_pushint64(lua_State *L, int64_t n)
    {
#if LUA_VERSION_NUM < 503
      int64_t v = n;
      if (n < 0)
      {
        v = n;
      }
      if (v < (1LL << 53LL))
      {
        lua_pushnumber(L, (lua_Number)n);
      }
      else
      {
        int64_t *o = (int64_t *)lua_newuserdata(L, sizeof(int64_t));
        *o = n;
#if LUA_VERSION_NUM == 501
        luaL_newmetatable(L, int64_metatable);
        lua_setmetatable(L, -2);
#else
        luaL_setmetatable(L, int64_metatable);
#endif
      }
#else
      lua_pushinteger(L, n);
#endif
      return;
    }

#if LUA_VERSION_NUM < 503

    static int new_int64(lua_State *L)
    {
      int top = lua_gettop(L);
      switch (top)
      {
      case 0:
      {
        lua_pushint64(L, 0);
        break;
      }
      case 1:
      {
        number64_type it = { et_int64_unknow };
        lua_tonumber64(L, 1, &it);
        if (it.type == et_int64_lua_number)
        {
          lua_pushint64(L, (int64_t)it.value.d64);
        }
        else if (it.type == et_int64_int64)
        {
          lua_pushint64(L, it.value.i64);
          break;
        }
        else if (it.type == et_int64_uint64)
        {
          if (it.value.u64 > (uint64_t)max_int64_integer)
          {
            luaL_error(L, "can't convert %ull to int64", it.value.u64);
          }
          lua_pushint64(L, (int64_t)it.value.u64);
          break;
        }
      }
      default:
      {
        luaL_error(L, "too many parameter to new int64");
        break;
      }
      }
      return 1;
    }

    static int lua_int64tostring(lua_State *L)
    {
      int64_t value = *(int64_t*)lua_touserdata(L, 1);
      char str[32] = { 0 };
      int sign = 0;
      size_t pos = 30;
      if (value < 0)
      {
        sign = 1;
        value = -value;
      }
      while (value)
      {
        char n = (char)(value % 10);
        value = value / 10;
        str[pos--] = '0' + n;
      }
      if (sign)
      {
        str[pos] = '-';
      }
      else
      {
        ++pos;
      }
      lua_pushstring(L, str + pos);
      return 1;
    }
#endif

    static int int64_eq(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.d64 == it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, it1.value.d64 == it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.d64 == it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform eq operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.i64 == it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, it1.value.i64 == it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.i64 == it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform eq operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.u64 == it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, it1.value.u64 == it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.u64 == it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform eq operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform eq operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_lt(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.d64 < it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, it1.value.d64 < it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.d64 < it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform lt operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.i64 < it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, it1.value.i64 < it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.i64 < (double)it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform lt operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.u64 < it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, (double)it1.value.u64 < it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.u64 < it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform lt operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform lt operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_le(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.d64 <= it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, it1.value.d64 <= it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.d64 <= it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform le operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.i64 <= it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, it1.value.i64 <= it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.i64 <= (double)it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform le operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushboolean(L, it1.value.u64 <= it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushboolean(L, (double)it1.value.u64 <= it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushboolean(L, it1.value.u64 <= it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform le operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform le operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_add(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushnumber(L, it1.value.d64 + it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushnumber(L, it1.value.d64 + it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushnumber(L, it1.value.d64 + it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform add operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushint64(L, it1.value.i64 + (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushint64(L, it1.value.i64 + (int64_t)it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushint64(L, it1.value.i64 + (int64_t)it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform add operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushuint64(L, it1.value.u64 + (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushuint64(L, it1.value.u64 + it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushuint64(L, it1.value.u64 + it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform add operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform add operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_sub(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushnumber(L, it1.value.d64 - it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushnumber(L, it1.value.d64 - it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushnumber(L, it1.value.d64 - it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform sub operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushint64(L, it1.value.i64 - (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushint64(L, it1.value.i64 - (int64_t)it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushint64(L, it1.value.i64 - (int64_t)it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform sub operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushuint64(L, it1.value.u64 - (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushuint64(L, it1.value.u64 - it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushuint64(L, it1.value.u64 - it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform sub operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform sub operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_mul(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushnumber(L, it1.value.d64 * it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushnumber(L, it1.value.d64 * it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushnumber(L, it1.value.d64 * it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform mul operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushint64(L, it1.value.i64 * (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushint64(L, it1.value.i64 * (int64_t)it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushint64(L, it1.value.i64 * (int64_t)it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform mul operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushuint64(L, it1.value.u64 * (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushuint64(L, it1.value.u64 * it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushuint64(L, it1.value.u64 * it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform mul operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform mul operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_div(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushnumber(L, it1.value.d64 - it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushnumber(L, it1.value.d64 - it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushnumber(L, it1.value.d64 - it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform sub operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushint64(L, it1.value.i64 / (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushint64(L, it1.value.i64 / (int64_t)it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushint64(L, it1.value.i64 / (int64_t)it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform div operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushuint64(L, it1.value.u64 / (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushuint64(L, it1.value.u64 / it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushuint64(L, it1.value.u64 / it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform div operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform div operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_mod(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushint64(L, (int64_t)it1.value.d64 % (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushint64(L, (int64_t)it1.value.d64 % it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushint64(L, (int64_t)it1.value.d64 % it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform mod operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushint64(L, it1.value.i64 % (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushint64(L, it1.value.i64 % it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushint64(L, it1.value.i64 % it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform mod operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushuint64(L, it1.value.u64 % (int64_t)it2.value.d64);
          break;
        }
        case et_int64_int64:
        {
          lua_pushuint64(L, it1.value.u64 % it2.value.i64);
          break;
        }
        case et_int64_uint64:
        {
          lua_pushuint64(L, it1.value.u64 % it2.value.u64);
          break;
        }
        default:
        {
          luaL_error(L, "can't perform mod operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform mod operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_pow(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow }, it2 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      lua_tonumber64(L, 2, &it2);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushnumber(L, pow(it1.value.d64, it2.value.d64));
          break;
        }
        case et_int64_int64:
        {
          lua_pushnumber(L, pow(it1.value.d64, (double)it2.value.i64));
          break;
        }
        case et_int64_uint64:
        {
          lua_pushnumber(L, pow(it1.value.d64, (double)it2.value.u64));
          break;
        }
        default:
        {
          luaL_error(L, "can't perform pow operator between number and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_int64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushint64(L, (int64_t)pow(it1.value.i64, it2.value.d64));
          break;
        }
        case et_int64_int64:
        {
          lua_pushint64(L, (int64_t)pow((double)it1.value.i64, (double)it2.value.i64));
          break;
        }
        case et_int64_uint64:
        {
          lua_pushint64(L, (int64_t)pow(it1.value.i64, it2.value.d64));
          break;
        }
        default:
        {
          luaL_error(L, "can't perform pow operator between int64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      case et_int64_uint64:
      {
        switch (it2.type)
        {
        case et_int64_lua_number:
        {
          lua_pushuint64(L, (uint64_t)pow(it1.value.u64, it2.value.d64));
          break;
        }
        case et_int64_int64:
        {
          lua_pushuint64(L, (uint64_t)pow((double)it1.value.u64, (double)it2.value.i64));
          break;
        }
        case et_int64_uint64:
        {
          lua_pushuint64(L, (uint64_t)pow((double)it1.value.u64, (double)it2.value.u64));
          break;
        }
        default:
        {
          luaL_error(L, "can't perform pow operator between uint64 and %s", lua_typename(L, 2));
        }
        }
        break;
      }
      default:
      {
        luaL_error(L, "can't perform pow operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

    static int int64_unm(lua_State *L)
    {
      number64_type it1 = { et_int64_unknow };
      lua_tonumber64(L, 1, &it1);
      switch (it1.type)
      {
      case et_int64_lua_number:
      {
        lua_pushnumber(L, -it1.value.d64);
        break;
      }
      case et_int64_int64:
      {
        lua_pushint64(L, -it1.value.i64);
        break;
      }
      case et_int64_uint64:
      {
        luaL_error(L, "can't perform unm operator at uint64");
        break;
      }
      default:
      {
        luaL_error(L, "can't perform unm operator at %s", lua_typename(L, 1));
      }
      }
      return 1;
    }

#if LUA_VERSION_NUM < 503
    static const luaL_Reg int64_meta[] =
    {
      { "__tostring", lua_int64tostring },
      { "__eq", int64_eq },
      { "__lt", int64_lt },
      { "__le", int64_le },
      { "__add", int64_add },
      { "__sub", int64_sub },
      { "__mul", int64_mul },
      { "__div", int64_div },
      { "__mod", int64_mod },
      { "__pow", int64_pow },
      { "__unm", int64_unm },
      { NULL, NULL }
    };

#endif

    static int new_uint64(lua_State *L)
    {
      int top = lua_gettop(L);
      switch (top)
      {
      case 0:
      {
        lua_pushuint64(L, 0);
        break;
      }
      case 1:
      {
        number64_type it = { et_int64_unknow };
        lua_tonumber64(L, 1, &it);
        if (it.type == et_int64_lua_number)
        {
          if (it.value.d64 < 0)
          {
            luaL_error(L, "can't convert %lf to int64", (uint64_t)it.value.d64);
          }
          lua_pushuint64(L, (uint64_t)it.value.d64);
        }
        else if (it.type == et_int64_int64)
        {
          if (it.value.i64 < 0)
          {
            luaL_error(L, "can't convert %ll to uint64", (uint64_t)it.value.i64);
          }
          lua_pushuint64(L, it.value.i64);
          break;
        }
        else if (it.type == et_int64_uint64)
        {
          lua_pushuint64(L, it.value.u64);
          break;
        }
      }
      default:
      {
        luaL_error(L, "too many parameter to new int64");
        break;
      }
      }
      return 1;
    }

    static int lua_uint64tostring(lua_State *L)
    {
      uint64_t value = *(uint64_t*)lua_touserdata(L, 1);
      char str[32] = { 0 };
      size_t pos = 30;
      while (value)
      {
        char n = (char)(value % 10);
        value = value / 10;
        str[pos--] = '0' + n;
      }
      ++pos;
      lua_pushstring(L, str + pos);
      return 1;
    }

    static const luaL_Reg uint64_meta[] =
    {
      { "__tostring", lua_uint64tostring },
      { "__eq", int64_eq },
      { "__lt", int64_lt },
      { "__le", int64_le },
      { "__add", int64_add },
      { "__sub", int64_sub },
      { "__mul", int64_mul },
      { "__div", int64_div },
      { "__mod", int64_mod },
      { "__pow", int64_pow },
      { "__unm", int64_unm },
      { NULL, NULL }
    };

#if LUA_VERSION_NUM == 501

    static void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
      luaL_checkstack(L, nup, "too many upvalues");
      for (; l->name != NULL; l++) {  /* fill the table with given functions */
        int i;
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
          lua_pushvalue(L, -nup);
        lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        lua_setfield(L, -(nup + 2), l->name);
      }
      lua_pop(L, nup);  /* remove upvalues */
    }

    ADATA_INLINE int init_lua_int64(lua_State *L)
    {
      static const luaL_Reg lib[] =
      {
        { "int64", new_int64 },
        { "uint64", new_uint64 },
        { NULL, NULL }
      };
      luaL_newmetatable(L, int64_metatable);
      luaL_setfuncs(L, int64_meta, 0);
      lua_pushvalue(L, -1);
      lua_setfield(L, -2, "__index");

      luaL_newmetatable(L, uint64_metatable);
      luaL_setfuncs(L, uint64_meta, 0);
      lua_pushvalue(L, -1);
      lua_setfield(L, -2, "__index");

      luaL_register(L, "int64", lib);
      return 1;
    }
#else
    static int regist_int64(lua_State * L)
    {
      luaL_checkversion(L);
      static const luaL_Reg lib[] =
      {
# if LUA_VERSION_NUM < 503
        { "int64", new_int64 },
# endif
        { "uint64", new_uint64 },
        { NULL, NULL }
      };
      luaL_newlib(L, lib);
      return 1;
    }

    ADATA_INLINE int init_lua_int64(lua_State *L)
    {
# if LUA_VERSION_NUM < 503
      luaL_newmetatable(L, int64_metatable);
      luaL_setfuncs(L, int64_meta, 0);
      lua_pushvalue(L, -1);
      lua_setfield(L, -2, "__index");
# endif
      luaL_newmetatable(L, uint64_metatable);
      luaL_setfuncs(L, uint64_meta, 0);
      lua_pushvalue(L, -1);
      lua_setfield(L, -2, "__index");

      luaL_requiref(L, "int64", regist_int64, 1);
      return 1;
    }
#endif
  }
}

#endif
