// (C) Copyright Ning Ding 2014.8
// lordoffox@gmail.com
// Distributed under the boost Software License, Version 1.0. (See accompany-
// ing file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef l_adata_corec_hpp
#define l_adata_corec_hpp

#include "adata.hpp"
#include "adata_int64.hpp"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace adata {
  namespace lua{

    static const char * zbuffer_metatable = "zbuf_meta";

    static int new_zbuf(lua_State * L)
    {
      lua_Integer size = lua_tointeger(L, 1);
      if (size < 0)
      {
        return luaL_error(L, "error buffer size %d", size);
      }
      if (size == 0)
      {
        size = 65535;
      }

      uint8_t* buffer = (uint8_t*)malloc(size);

      void * obj = lua_newuserdata(L, sizeof(adata::zero_copy_buffer));
      
      adata::zero_copy_buffer * zbuf = new (obj)adata::zero_copy_buffer;
      zbuf->set_write(buffer, size);
#if LUA_VERSION_NUM == 501
      luaL_newmetatable(L, zbuffer_metatable);
      lua_setmetatable(L, -2);
#else
      luaL_setmetatable(L, zbuffer_metatable);
#endif
      return 1;
    }

    ADATA_INLINE void _reset_buf(adata::zero_copy_buffer& zbuf)
    {
      if (zbuf.write_data() != 0)
      {
        // Nous Xiong: change to free, for resize realloc
        std::free((void*)zbuf.write_data());
        zbuf.set_write((uint8_t *)NULL, 0);
      }
    }

    static int del_zbuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      _reset_buf(*zbuf);
      return 1;
    }

    static int resize_zuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      lua_Integer size = lua_tointeger(L, 2);
      if (size <= 0)
      {
        return luaL_error(L, "error buffer size %d", size);
      }

      // Nous Xiong: change to realloc to improvment
      void* p = (void*)zbuf->write_data();
      uint8_t* buffer = (uint8_t*)std::realloc(p, size);
      if (buffer == 0)
      {
        return luaL_error(L, "buffer alloc failed");
      }

      zbuf->set_write(buffer, size);
      return 1;
    }

    static int clear_zuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      zbuf->clear();
      return 1;
    }

    static int set_error_zuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      lua_Integer ec = lua_tointeger(L, 2);
      zbuf->set_error_code((adata::error_code_t)ec);
      return 1;
    }

    static int trace_error_zuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      const char * info = (const char *)lua_touserdata(L, 2);
      lua_Integer offset = lua_tointeger(L, 3);
      lua_Integer idx = lua_tointeger(L, 4);
      if (idx == 0)
      {
        idx = -1;
      }
      zbuf->trace_error(info + offset, (int)idx);
      return 1;
    }

    static int trace_info_zuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      std::string info;
      zbuf->get_trace_error_info(info);
      lua_pushlstring(L, info.data(), info.length());
      return 1;
    }

    static int get_read_length(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      lua_pushinteger(L, zbuf->read_length());
      return 1;
    }

    static int get_write_length(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      lua_pushinteger(L, zbuf->write_length());
      return 1;
    }

    static int get_write_buf_zuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      lua_pushlstring(L, zbuf->write_data(), zbuf->write_length());
      lua_Integer dont_clear = lua_tointeger(L, 2);
      if (!dont_clear)
      {
        zbuf->clear();
      }
      return 1;
    }

    static int set_read_buf_zuf(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, 1);
      if (NULL == zbuf)
      {
        return luaL_error(L, "arg 1 must be zbuf!");
      }
      size_t len;
      const char * str = lua_tolstring(L, 2, &len);
      size_t nlen = lua_tointeger(L, 3);
      if (nlen > 0)
      {
        len = nlen;
      }
      zbuf->set_read(str, len);
      return 1;
    }

    ADATA_INLINE adata::zero_copy_buffer * _get_zbuf_arg(lua_State * L, int idx)
    {
      adata::zero_copy_buffer * zbuf = (adata::zero_copy_buffer *)lua_touserdata(L, idx);
      if (NULL == zbuf)
      {
        luaL_error(L, "arg 1 must be zbuf!");
      }
      return zbuf;
    }

    template <typename T>
    ADATA_INLINE int read_fix_value(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      T v;
      adata::fix_read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushinteger(L, v);
      return 2;
    }

    template <typename T>
    ADATA_INLINE int read_value(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      T v;
      adata::read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushinteger(L, v);
      return 2;
    }

    static int read_fix_int8(lua_State * L)
    {
      return read_fix_value<int8_t>(L);
    }

    static int read_fix_uint8(lua_State * L)
    {
      return read_fix_value<uint8_t>(L);
    }

    static int read_fix_int16(lua_State * L)
    {
      return read_fix_value<int16_t>(L);
    }

    static int read_fix_uint16(lua_State * L)
    {
      return read_fix_value<uint16_t>(L);
    }

    static int read_fix_int32(lua_State * L)
    {
      return read_fix_value<int32_t>(L);
    }

    static int read_fix_uint32(lua_State * L)
    {
      return read_fix_value<uint32_t>(L);
    }

#if LUA_VERSION_NUM < 503
    static int read_fix_int64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      int64_t v;
      adata::fix_read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushint64(L, v);
      return 2;
    }
#else
    static int read_fix_int64(lua_State * L)
    {
      return read_fix_value<int64_t>(L);
    }
#endif

    static int read_fix_uint64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      uint64_t v;
      adata::fix_read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushuint64(L, v);
      return 2;
    }

    static int read_int8(lua_State * L)
    {
      return read_value<int8_t>(L);
    }

    static int read_uint8(lua_State * L)
    {
      return read_value<uint8_t>(L);
    }

    static int read_int16(lua_State * L)
    {
      return read_value<int16_t>(L);
    }

    static int read_uint16(lua_State * L)
    {
      return read_value<uint16_t>(L);
    }

    static int read_int32(lua_State * L)
    {
      return read_value<int32_t>(L);
    }

    static int read_uint32(lua_State * L)
    {
      return read_value<uint32_t>(L);
    }

#if LUA_VERSION_NUM < 503
    static int read_int64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      int64_t v;
      adata::read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushint64(L, v);
      return 2;
    }
#else
    static int read_int64(lua_State * L)
    {
      return read_value<int64_t>(L);
    }
#endif

    static int read_uint64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      uint64_t v;
      adata::read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushuint64(L, v);
      return 2;
    }

    static int read_tag(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      uint64_t v;
      adata::read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushint64(L, (int64_t)v);
      return 2;
    }

    static int read_float32(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      float v;
      adata::read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushnumber(L, v);
      return 2;
    }

    static int read_float64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      double v;
      adata::read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      lua_pushnumber(L, v);
      return 2;
    }

    static int read_str(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      uint32_t slen = 0;
      adata::read(*zbuf, slen);
      lua_Integer len = lua_tointeger(L, 2);
      if (len > 0 && len < (lua_Integer)slen)
      {
        zbuf->set_error_code(sequence_length_overflow);
        slen = 0;
      }
      const char * str = (const char *)zbuf->skip_read(slen);
      if (zbuf->error())
      {
        slen = 0;
      }
      lua_pushinteger(L, zbuf->error_code());
      lua_pushlstring(L, str, slen);
      return 2;
    }

    template <typename T>
    ADATA_INLINE int skip_read_fix_value(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      zbuf->skip_read(sizeof(T));
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    template <typename T>
    ADATA_INLINE int skip_read_value(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      T* v = 0;
      adata::skip_read(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int skip_read(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      lua_Integer len = lua_tointeger(L, 2);
      zbuf->skip_read((size_t)len);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int skip_read_fix_int8(lua_State * L)
    {
      return skip_read_fix_value<int8_t>(L);
    }

    static int skip_read_fix_uint8(lua_State * L)
    {
      return skip_read_fix_value<uint8_t>(L);
    }

    static int skip_read_fix_int16(lua_State * L)
    {
      return skip_read_fix_value<int16_t>(L);
    }

    static int skip_read_fix_uint16(lua_State * L)
    {
      return skip_read_fix_value<uint16_t>(L);
    }

    static int skip_read_fix_int32(lua_State * L)
    {
      return skip_read_fix_value<int32_t>(L);
    }

    static int skip_read_fix_uint32(lua_State * L)
    {
      return skip_read_fix_value<uint32_t>(L);
    }

    static int skip_read_fix_int64(lua_State * L)
    {
      return skip_read_fix_value<int64_t>(L);
    }

    static int skip_read_fix_uint64(lua_State * L)
    {
      return skip_read_fix_value<uint64_t>(L);
    }

    static int skip_read_int8(lua_State * L)
    {
      return skip_read_value<int8_t>(L);
    }

    static int skip_read_uint8(lua_State * L)
    {
      return skip_read_value<uint8_t>(L);
    }

    static int skip_read_int16(lua_State * L)
    {
      return skip_read_value<int16_t>(L);
    }

    static int skip_read_uint16(lua_State * L)
    {
      return skip_read_value<uint16_t>(L);
    }

    static int skip_read_int32(lua_State * L)
    {
      return skip_read_value<int32_t>(L);
    }

    static int skip_read_uint32(lua_State * L)
    {
      return skip_read_value<uint32_t>(L);
    }

    static int skip_read_int64(lua_State * L)
    {
      return skip_read_value<int64_t>(L);
    }

    static int skip_read_uint64(lua_State * L)
    {
      return skip_read_value<uint64_t>(L);
    }

    static int skip_read_float32(lua_State * L)
    {
      return skip_read_value<float>(L);
    }

    static int skip_read_float64(lua_State * L)
    {
      return skip_read_value<double>(L);
    }

    static int skip_read_str(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      uint32_t slen = 0;
      adata::read(*zbuf, slen);
      lua_Integer len = lua_tointeger(L, 2);
      if (len > 0 && len < (lua_Integer)slen)
      {
        zbuf->set_error_code(sequence_length_overflow);
        slen = 0;
      }
      zbuf->skip_read(slen);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    template<typename T>
    ADATA_INLINE void get_lua_number64(lua_State * L, int idx, T& v)
    {
      number64_type it;
      lua_tonumber64(L, idx, &it);
      switch (it.type)
      {
      case et_int64_lua_number:
      {
        v = (T)it.value.d64;
        break;
      }
      case et_int64_int64:
      {
        v = (T)it.value.i64;
        break;
      }
      case et_int64_uint64:
      {
        v = (T)it.value.u64;
        break;
      }
      default: v = 0;
      }
    }

    template <typename T>
    ADATA_INLINE int write_fix_value(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      T v = (T)lua_tointeger(L, 2);
      adata::fix_write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    template <typename T>
    ADATA_INLINE int write_value(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      T v = (T)lua_tointeger(L, 2);
      adata::write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int write_fix_int8(lua_State * L)
    {
      return write_fix_value<int8_t>(L);
    }

    static int write_fix_uint8(lua_State * L)
    {
      return write_fix_value<uint8_t>(L);
    }

    static int write_fix_int16(lua_State * L)
    {
      return write_fix_value<int16_t>(L);
    }

    static int write_fix_uint16(lua_State * L)
    {
      return write_fix_value<uint16_t>(L);
    }

    static int write_fix_int32(lua_State * L)
    {
      return write_fix_value<int32_t>(L);
    }

    static int write_fix_uint32(lua_State * L)
    {
      return write_fix_value<uint32_t>(L);
    }

#if LUA_VERSION_NUM < 503
    static int write_fix_int64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      int64_t v;
      get_lua_number64(L, 2, v);
      adata::fix_write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }
#else
    static int write_fix_int64(lua_State * L)
    {
      return write_fix_value<int64_t>(L);
    }
#endif

    static int write_fix_uint64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      uint64_t v;
      get_lua_number64(L, 2, v);
      adata::fix_write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int write_int8(lua_State * L)
    {
      return write_value<int8_t>(L);
    }

    static int write_uint8(lua_State * L)
    {
      return write_value<uint8_t>(L);
    }

    static int write_int16(lua_State * L)
    {
      return write_value<int16_t>(L);
    }

    static int write_uint16(lua_State * L)
    {
      return write_value<uint16_t>(L);
    }

    static int write_int32(lua_State * L)
    {
      return write_value<int32_t>(L);
    }

    static int write_uint32(lua_State * L)
    {
      return write_value<uint32_t>(L);
    }

#if LUA_VERSION_NUM < 503
    static int write_int64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      int64_t v;
      get_lua_number64(L, 2, v);
      adata::write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }
#else
    static int write_int64(lua_State * L)
    {
      return write_value<int64_t>(L);
    }
#endif

#if LUA_VERSION_NUM < 503
    static int write_tag(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      int64_t v;
      get_lua_number64(L, 2, v);
      adata::write(*zbuf, (uint64_t)v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }
#else
    static int write_tag(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      int64_t v = lua_tointeger(L, 2);
      adata::write(*zbuf, (uint64_t)v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }
#endif

    static int write_uint64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      uint64_t v;
      get_lua_number64(L, 2, v);
      adata::write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int write_float32(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      float v = (float)lua_tonumber(L, 2);
      adata::write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int write_float64(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      double v = lua_tonumber(L, 2);
      adata::write(*zbuf, v);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int write_str(lua_State * L)
    {
      adata::zero_copy_buffer * zbuf = _get_zbuf_arg(L, 1);
      size_t slen = 0;
      const char * str = lua_tolstring(L, 2, &slen);
      lua_Integer len = lua_tointeger(L, 3);
      if (len > 0 && len < (lua_Integer)slen)
      {
        zbuf->set_error_code(sequence_length_overflow);
        slen = 0;
      }
      adata::write(*zbuf, (uint32_t)slen);
      zbuf->write(str, slen);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    template <typename T>
    ADATA_INLINE int size_of_fix_value(lua_State * L)
    {
      lua_pushinteger(L, adata::fix_size_of(T()));
      return 1;
    }

    template <typename T>
    ADATA_INLINE int size_of_value(lua_State * L)
    {
      T v = (T)lua_tointeger(L, 1);
      int32_t s = adata::size_of(v);
      lua_pushinteger(L, s);
      return 1;
    }

    static int size_of_fix_int8(lua_State * L)
    {
      return size_of_fix_value<int8_t>(L);
    }

    static int size_of_fix_uint8(lua_State * L)
    {
      return size_of_fix_value<uint8_t>(L);
    }

    static int size_of_fix_int16(lua_State * L)
    {
      return size_of_fix_value<int16_t>(L);
    }

    static int size_of_fix_uint16(lua_State * L)
    {
      return size_of_fix_value<uint16_t>(L);
    }

    static int size_of_fix_int32(lua_State * L)
    {
      return size_of_fix_value<int32_t>(L);
    }

    static int size_of_fix_uint32(lua_State * L)
    {
      return size_of_fix_value<uint32_t>(L);
    }

    static int size_of_fix_int64(lua_State * L)
    {
      return size_of_fix_value<int64_t>(L);
    }

    static int size_of_fix_uint64(lua_State * L)
    {
      return size_of_fix_value<uint64_t>(L);
    }

    static int size_of_int8(lua_State * L)
    {
      return size_of_value<int8_t>(L);
    }

    static int size_of_uint8(lua_State * L)
    {
      return size_of_value<uint8_t>(L);
    }

    static int size_of_int16(lua_State * L)
    {
      return size_of_value<int16_t>(L);
    }

    static int size_of_uint16(lua_State * L)
    {
      return size_of_value<uint16_t>(L);
    }

    static int size_of_int32(lua_State * L)
    {
      return size_of_value<int32_t>(L);
    }

    static int size_of_uint32(lua_State * L)
    {
      return size_of_value<uint32_t>(L);
    }

#if LUA_VERSION_NUM < 503
    static int size_of_int64(lua_State * L)
    {
      int64_t v;
      get_lua_number64(L, 1, v);
      int32_t s = adata::size_of(v);
      lua_pushinteger(L, s);
      return 1;
    }
#else
    static int size_of_int64(lua_State * L)
    {
      return size_of_value<int64_t>(L);
    }
#endif

    static int size_of_uint64(lua_State * L)
    {
      uint64_t v;
      get_lua_number64(L, 1, v);
      int32_t s = adata::size_of(v);
      lua_pushinteger(L, s);
      return 1;
    }

    static int size_of_float32(lua_State * L)
    {
      int32_t s = adata::size_of(float());
      lua_pushinteger(L, s);
      return 1;
    }

    static int size_of_float64(lua_State * L)
    {
      int32_t s = adata::size_of(double());
      lua_pushinteger(L, s);
      return 1;
    }

    static int size_of_str(lua_State * L)
    {
      size_t len;
      lua_tolstring(L, 1, &len);
      int32_t s = adata::size_of((int32_t)len);
      s += (int32_t)len;
      lua_pushinteger(L, s);
      return 1;
    }

    enum
    {
      adata_et_unknow,
      adata_et_fix_int8,
      adata_et_fix_uint8,
      adata_et_fix_int16,
      adata_et_fix_uint16,
      adata_et_fix_int32,
      adata_et_fix_uint32,
      adata_et_fix_int64,
      adata_et_fix_uint64,
      adata_et_int8,
      adata_et_uint8,
      adata_et_int16,
      adata_et_uint16,
      adata_et_int32,
      adata_et_uint32,
      adata_et_int64,
      adata_et_uint64,
      adata_et_float32,
      adata_et_float64,
      adata_et_string,
      adata_et_list,
      adata_et_map,
      adata_et_type
    };

    typedef struct adata_member adata_member;

    typedef struct adata_type
    {
      adata_member * members;
      int32_t member_count;
      int32_t mt_idx;
      char * name;
    }adata_type;

    typedef struct adata_paramter_type
    {
      int32_t type;
      int32_t size;
      char * type_name;
      adata_type * type_define;
    }adata_paramter_type;

    typedef struct adata_member
    {
      int32_t type;
      int32_t del;
      int32_t size;
      int32_t field_idx;
      char * name;
      char * type_name;
      adata_type * type_define;
      adata_paramter_type * paramter_type[2];
    }adata_member;

    struct load_contex
    {
      enum
      {
        filename_idx = 1,
        str_pool_idx,
        str_idx_idx,
        type_table_idx,
        mt_list_idx,
      };
      int namespace_table_idx;
      std::string ns;
      std::vector<char*> namespace_str_pool;
      std::vector<int32_t> namespace_str_idx;
      std::vector<adata_type *> types;
      load_contex()
        :namespace_table_idx(-1)
      {}
    };

    ADATA_INLINE void decode_default_value(lua_State * L, adata::zero_copy_buffer& buf, adata_member * mb, int32_t typename_sid , int member_idx, int construct_list_idx)
    {
      switch (mb->type)
      {
      case adata_et_fix_int8:
      case adata_et_fix_int16:
      case adata_et_fix_int32:
      case adata_et_fix_int64:
      case adata_et_int8:
      case adata_et_int16:
      case adata_et_int32:
      case adata_et_int64:
      {
        int64_t value = 0;
        adata::read(buf, value);
        lua_pushint64(L, value);
        lua_rawseti(L, construct_list_idx, member_idx);
        break;
      }
      case adata_et_fix_uint8:
      case adata_et_fix_uint16:
      case adata_et_fix_uint32:
      case adata_et_fix_uint64:
      case adata_et_uint8:
      case adata_et_uint16:
      case adata_et_uint32:
      case adata_et_uint64:
      {
        uint64_t value = 0;
        adata::read(buf, value);
        lua_pushuint64(L, value);
        lua_rawseti(L, construct_list_idx, member_idx);
        break;
      }
      case adata_et_float32:
      {
        float value = 0;
        adata::read(buf, value);
        lua_pushnumber(L, value);
        lua_rawseti(L, construct_list_idx, member_idx);
        break;
      }
      case adata_et_float64:
      {
        double value = 0;
        adata::read(buf, value);
        lua_pushnumber(L, value);
        lua_rawseti(L, construct_list_idx, member_idx);
        break;
      }
      case adata_et_string:
      {
        lua_pushstring(L, "");
        lua_rawseti(L, construct_list_idx, member_idx);
        break;
      }
      case adata_et_type:
      {
        lua_rawgeti(L, load_contex::str_idx_idx, typename_sid);
        lua_rawseti(L, construct_list_idx, member_idx);
        break;
      }
      default:
      {
        lua_pushnil(L);
        lua_rawseti(L, construct_list_idx, member_idx);
        break;
      }
      }
    }

    ADATA_INLINE int32_t get_lua_len(lua_State * L, int idx)
    {
#if LUA_VERSION_NUM == 501
      return (int32_t)lua_objlen(L, idx);
#else
      return (int32_t)lua_rawlen(L, idx);
#endif
    }

    ADATA_INLINE int load_type(lua_State * L, adata::zero_copy_buffer& buf, load_contex& context)
    {
      //type { layout , name , {field list} , { construct list }  ,  metatable }
      enum
      {
        idx_layout = 1,
        idx_name,
        idx_field_list,
        idx_construct_list,
        idx_metatable
      };

      lua_createtable(L, 6, 0);             //define type
      int type_idx = lua_gettop(L);

      uint32_t member_count;
      uint32_t param_type_count;
      read(buf, member_count);
      read(buf, param_type_count);
      uint32_t total_size = sizeof(adata_type) + sizeof(adata_member)*member_count + sizeof(adata_paramter_type)*param_type_count;
      char * type_buffer = (char *)lua_newuserdata(L, total_size);
      adata_type * type = (adata_type *)type_buffer;
      char * adata_member_buffer = type_buffer + sizeof(adata_type);
      char * adata_paramter_type_buffer = adata_member_buffer + sizeof(adata_member)*member_count;

      lua_pushvalue(L, -1);                   //copy layout for register
      lua_rawseti(L, type_idx, idx_layout);   //set type layout

      context.types.push_back(type);

      type->member_count = member_count;
      type->members = (adata_member *)adata_member_buffer;

      int32_t type_name_sid;

      read(buf, type_name_sid);
      int member_list_count = 1;

      type->name = context.namespace_str_pool[type_name_sid];
      type_name_sid = context.namespace_str_idx[type_name_sid];
      lua_rawgeti(L, load_contex::str_idx_idx, type_name_sid);
      lua_rawseti(L, type_idx, idx_name);   //set type name

      std::string type_fullname = context.ns;
      type_fullname.append(".");
      type_fullname.append(type->name);
      lua_pushlstring(L, type_fullname.data(), type_fullname.length());
      lua_pushvalue(L, -2);
      lua_settable(L, context.type_table_idx);  //register type full name layout to global table
      lua_pop(L, 1);
      lua_createtable(L, member_count, 0);  // field name list
      int field_list_idx = lua_gettop(L);
      lua_createtable(L, member_count, 0);  // construct list
      int construct_list_idx = lua_gettop(L);

      for (uint32_t i = 0; i < member_count; ++i)
      {
        adata_member * mb = &type->members[i];
        mb->paramter_type[0] = NULL;
        mb->paramter_type[1] = NULL;
        int32_t member_name_sid;
        read(buf, member_name_sid);
        mb->name = context.namespace_str_pool[member_name_sid];
        mb->field_idx = context.namespace_str_idx[member_name_sid];
        read(buf, mb->type);
        int32_t member_typename_sid = 0;

        if (mb->type == adata_et_type)
        {
          read(buf, member_typename_sid);
          mb->type_name = context.namespace_str_pool[member_typename_sid];
          member_typename_sid = context.namespace_str_idx[member_typename_sid];

          int32_t namespace_idx = 0;
          read(buf, namespace_idx);
          if (namespace_idx != -1)
          {
            mb->type_define = context.types[namespace_idx];
          }
          else
          {
            lua_rawgeti(L, load_contex::str_idx_idx, member_typename_sid);
            lua_gettable(L, load_contex::type_table_idx);
            mb->type_define = (adata_type *)lua_touserdata(L, -1);
            lua_pop(L,1);
            if (NULL == mb->type_define)
            {
              luaL_error(L, "undefined type %s", mb->type_name);
              return 0;
            }
          }
        }
        else
        {
          mb->type_define = NULL;
        }
        read(buf, mb->del);
        if (mb->del == 0)
        {
          lua_rawgeti(L, load_contex::str_idx_idx, mb->field_idx);
          lua_rawseti(L, field_list_idx, member_list_count);
          decode_default_value(L, buf, mb, member_typename_sid , member_list_count++, construct_list_idx);
        }
        read(buf, mb->size);
        read(buf, param_type_count);
        for (uint32_t p = 0; p < param_type_count; ++p)
        {
          mb->paramter_type[p] = (adata_paramter_type*)adata_paramter_type_buffer;
          adata_paramter_type_buffer += sizeof(adata_paramter_type);
          adata_paramter_type * ptype = mb->paramter_type[p];
          read(buf, ptype->type);
          if (ptype->type == adata_et_type)
          {
            int ptype_typename_sid;

            read(buf, ptype_typename_sid);
            ptype->type_name = context.namespace_str_pool[ptype_typename_sid];
            ptype_typename_sid = context.namespace_str_idx[ptype_typename_sid];
            int32_t namespace_idx = 0;
            read(buf, namespace_idx);
            if (namespace_idx != -1)
            {
              ptype->type_define = context.types[namespace_idx];
            }
            else
            {
              lua_rawgeti(L, load_contex::str_idx_idx, ptype_typename_sid);
              lua_gettable(L, load_contex::type_table_idx);
              ptype->type_define = (adata_type *)lua_touserdata(L, -1);
              lua_pop(L, 1);
              if (NULL == ptype->type_define)
              {
                luaL_error(L, "undefined type %s", ptype->type_name);
                return 0;
              }
            }
          }
          else
          {
            ptype->type_define = NULL;
          }
          read(buf, ptype->size);
        }
      }
      lua_createtable(L,4,0);
      lua_pushvalue(L, -1);
      int32_t mt_len = 0;
      mt_len = get_lua_len(L, load_contex::mt_list_idx) + 1;
      lua_rawseti(L, load_contex::mt_list_idx, mt_len);
      type->mt_idx = mt_len;                              //assign metatable to type layout
      lua_pushvalue(L, -1);
      type_fullname = "ad.mt." + type_fullname;
      type_fullname.append("\0");
      lua_setfield(L, LUA_REGISTRYINDEX, type_fullname.c_str());  //register global metatable table for cpp2lua
      lua_rawseti(L, type_idx, idx_metatable);
      lua_rawseti(L, type_idx, idx_construct_list);
      lua_rawseti(L, type_idx, idx_field_list);
      return 1;
    }

    ADATA_INLINE void load_namespace(lua_State * L, adata::zero_copy_buffer& buf, load_contex& context)
    {
      //{ namespace , {type...} }
      lua_createtable(L, 2, 0);
      context.namespace_table_idx = lua_gettop(L);
      adata::read(buf, context.ns);
      lua_pushlstring(L, context.ns.data(), context.ns.length());
      lua_rawseti(L, context.namespace_table_idx, 1);
      uint32_t string_pool_count = 0;
      adata::read(buf, string_pool_count);
      std::string pool_value;
      context.namespace_str_pool.clear();
      context.types.clear();
      int32_t str_idx_len = get_lua_len(L, load_contex::str_idx_idx) + 1;

      for (uint32_t i = 0; i < string_pool_count; ++i)
      {
        adata::read(buf, pool_value);
        lua_pushlstring(L, pool_value.data(), pool_value.length());
        lua_pushvalue(L, -1);
        lua_pushvalue(L, -1);
        char * str = (char *)lua_tostring(L, -1);
        context.namespace_str_pool.push_back(str);
        lua_gettable(L, load_contex::str_pool_idx);
        if (lua_isnil(L, -1))
        {
          lua_pop(L, 1);
          lua_pushinteger(L, str_idx_len);
          lua_settable(L, load_contex::str_pool_idx);
          lua_rawseti(L, load_contex::str_idx_idx, str_idx_len);
          context.namespace_str_idx.push_back(str_idx_len++);
        }
        else
        {
          int32_t str_idx = (int32_t)lua_tointeger(L, -1);
          context.namespace_str_idx.push_back(str_idx);
          lua_pop(L, 3);
        }
      }
      uint32_t type_count = 0;
      adata::read(buf, type_count);
      lua_createtable(L, type_count, 0);
      int type_list_idx = lua_gettop(L);
      for (uint32_t i = 0; i < type_count; ++i)
      {
        load_type(L, buf, context);
        lua_rawseti(L, type_list_idx, i + 1);
      }
      lua_rawseti(L, context.namespace_table_idx, 2);
    }

    struct buffer_guard
    {
      buffer_guard(char * buffer)
        :buffer_(buffer)
      {}
      ~buffer_guard()
      {
        std::free(buffer_);
      }
    private:
      char * buffer_;
    };

    static int lua_load_pack(lua_State * L)
    {
      const char * filename = lua_tostring(L, load_contex::filename_idx);
      FILE * fp = std::fopen(filename, "rb");
      if (NULL == fp)
      {
        luaL_error(L, "can't open %s", filename);
        return 0;
      }
      std::fseek(fp, 0, SEEK_END);
      size_t len = std::ftell(fp);
      char * layout_buffer = (char *)std::malloc(len);
      buffer_guard bg(layout_buffer);
      std::fseek(fp, 0, SEEK_SET);
      std::fread(layout_buffer, len, 1, fp);
      fclose(fp);

      zero_copy_buffer buf;
      buf.set_read(layout_buffer, len);
      int32_t count = 0;
      adata::read(buf, count);
      lua_createtable(L, count, 0);
      int namespace_list_idx = lua_gettop(L);
      for (int32_t i = 0; i < count; ++i)
      {
        load_contex contex;
        load_namespace(L, buf, contex);
        lua_rawseti(L, namespace_list_idx, i + 1);
      }
      return 1;
    }

    inline bool set_metatable(lua_State * L, const char * name)
    {
      lua_getfield(L, LUA_REGISTRYINDEX, name);
      if (lua_type(L, -1) != LUA_TNIL)
      {
        lua_setmetatable(L, -2);
        return true;
      }
      lua_pop(L, 1);
      return false;
    }

    static int skip_read_type(lua_State *L, zero_copy_buffer * buf, adata_type * type);

    static ADATA_INLINE int skip_read_value(lua_State *L, zero_copy_buffer * buf, int type, int size, adata_type * type_define)
    {
      (size);
      switch (type)
      {
      case adata_et_fix_int8: { buf->skip_read(1); break; }
      case adata_et_fix_uint8:{ buf->skip_read(1); break; }
      case adata_et_fix_int16:{ buf->skip_read(2); break; }
      case adata_et_fix_uint16:{ buf->skip_read(2); break; }
      case adata_et_fix_int32:{ buf->skip_read(4); break; }
      case adata_et_fix_uint32:{ buf->skip_read(4); break; }
      case adata_et_fix_int64:{ buf->skip_read(8); break; }
      case adata_et_fix_uint64:{ buf->skip_read(8); break; }
      case adata_et_int8:{ adata::skip_read(*buf, (int8_t*)0); break; }
      case adata_et_uint8:{ adata::skip_read(*buf, (uint8_t*)0); break; }
      case adata_et_int16:{ adata::skip_read(*buf, (int16_t*)0); break; }
      case adata_et_uint16:{ adata::skip_read(*buf, (uint16_t*)0); break; }
      case adata_et_int32:{ adata::skip_read(*buf, (int32_t*)0); break; }
      case adata_et_uint32:{ adata::skip_read(*buf, (uint32_t*)0); break; }
      case adata_et_int64:{ adata::skip_read(*buf, (int64_t*)0); break; }
      case adata_et_uint64:{ adata::skip_read(*buf, (uint64_t*)0); break; }
      case adata_et_float32:{ buf->skip_read(4); break; }
      case adata_et_float64:{ buf->skip_read(8); break; }
      case adata_et_string:
      {
        uint32_t len = 0;
        adata::read(*buf, len);
        buf->skip_read(len);
        break;
      }
      case adata_et_type:
      {
        if (type_define)
        {
          skip_read_type(L, buf, type_define);
        }
        else
        {
          buf->set_error_code(undefined_member_protocol_not_compatible);
        }
        break;
      }
      default:
      {
        buf->set_error_code(undefined_member_protocol_not_compatible);
      }
      }
      return 0;
    }

    static int skip_read_member(lua_State *L, zero_copy_buffer * buf, adata_member * mb)
    {
      if (mb->type == adata_et_list)
      {
        uint32_t len = 0;
        adata::read(*buf, len);
        adata_paramter_type * ptype = mb->paramter_type[0];
        for (uint32_t i = 1; i <= len; ++i)
        {
          lua_rawgeti(L, -1, len);
          skip_read_value(L, buf, ptype->type, ptype->size, ptype->type_define);
          if (buf->error()) { buf->trace_error(mb->name, i); return 0; }
          lua_pop(L, 1);
        }
      }
      else if (mb->type == adata_et_map)
      {
        uint32_t len = 0;
        adata::read(*buf, len);
        adata_paramter_type * ptype1 = mb->paramter_type[0];
        adata_paramter_type * ptype2 = mb->paramter_type[1];
        for (uint32_t i = 1; i <= len; ++i)
        {
          lua_rawgeti(L, -1, len);
          lua_pushvalue(L, -2);
          skip_read_value(L, buf, ptype1->type, ptype1->size, ptype1->type_define);
          if (buf->error()) { buf->trace_error(mb->name, i); return 0; }
          lua_pop(L, 1);
          skip_read_value(L, buf, ptype2->type, ptype2->size, ptype2->type_define);
          if (buf->error()) { buf->trace_error(mb->name, i); return 0; }
          lua_pop(L, 1);
        }
      }
      else
      {
        skip_read_value(L, buf, mb->type, mb->size, mb->type_define);
        if (buf->error()) { buf->trace_error(mb->name, -1); return 0; }
      }
      return 1;
    }

    static int skip_read_type(lua_State *L, zero_copy_buffer * buf, adata_type * type)
    {
      (L);
      (type);
      adata::skip_read_compatible(*buf);
      if (buf->error())
      {
        return 0;
      }
      return 1;
    }

    template<typename ty>
    ADATA_INLINE void fix_read_and_push_value(lua_State *L, zero_copy_buffer * buf)
    {
      ty v;
      adata::fix_read(*buf, v);
      lua_pushinteger(L, v);
    }

    template<>
    ADATA_INLINE void fix_read_and_push_value<int64_t>(lua_State *L, zero_copy_buffer * buf)
    {
      int64_t v;
      adata::fix_read(*buf, v);
      lua_pushint64(L, v);
    }

    template<>
    ADATA_INLINE void fix_read_and_push_value<uint64_t>(lua_State *L, zero_copy_buffer * buf)
    {
      uint64_t v;
      adata::fix_read(*buf, v);
      lua_pushuint64(L, v);
    }

    template<typename ty>
    ADATA_INLINE void read_and_push_value(lua_State *L, zero_copy_buffer * buf)
    {
      ty v;
      adata::read(*buf, v);
      lua_pushinteger(L, v);
    }

    template<>
    ADATA_INLINE void read_and_push_value<float>(lua_State *L, zero_copy_buffer * buf)
    {
      float v;
      adata::read(*buf, v);
      lua_pushnumber(L, v);
    }

    template<>
    ADATA_INLINE void read_and_push_value<double>(lua_State *L, zero_copy_buffer * buf)
    {
      double v;
      adata::read(*buf, v);
      lua_pushnumber(L, v);
    }

    template<>
    ADATA_INLINE void read_and_push_value<int64_t>(lua_State *L, zero_copy_buffer * buf)
    {
      int64_t v;
      adata::read(*buf, v);
      lua_pushint64(L, v);
    }

    template<>
    ADATA_INLINE void read_and_push_value<uint64_t>(lua_State *L, zero_copy_buffer * buf)
    {
      uint64_t v;
      adata::read(*buf, v);
      lua_pushuint64(L, v);
    }

    static int read_type(lua_State *L, zero_copy_buffer * buf, adata_type * type, bool create = true);

    static ADATA_INLINE int read_string(lua_State *L, zero_copy_buffer * buf, int sz)
    {
      uint32_t len = 0;
      adata::read(*buf, len);
      if (buf->error()) { return 0; }
      if (sz > 0 && (int)len > sz)
      {
        buf->set_error_code(number_of_element_not_macth);
        return 0;
      }
      char * str = (char*)buf->skip_read(len);
      if (buf->bad())
      {
        buf->set_error_code(stream_buffer_overflow);
        return 0;
      }
      lua_pushlstring(L, str, len);
      return 1;
    }

    static ADATA_INLINE int read_value(lua_State *L, zero_copy_buffer * buf, int type, int size, adata_type * type_define)
    {
      switch (type)
      {
      case adata_et_fix_int8:{ fix_read_and_push_value<int8_t>(L, buf); break; }
      case adata_et_fix_uint8:{ fix_read_and_push_value<uint8_t>(L, buf); break; }
      case adata_et_fix_int16:{ fix_read_and_push_value<int16_t>(L, buf); break; }
      case adata_et_fix_uint16:{ fix_read_and_push_value<uint16_t>(L, buf); break; }
      case adata_et_fix_int32:{ fix_read_and_push_value<int32_t>(L, buf); break; }
      case adata_et_fix_uint32:{ fix_read_and_push_value<uint32_t>(L, buf); break; }
      case adata_et_fix_int64:{ fix_read_and_push_value<int64_t>(L, buf); break; }
      case adata_et_fix_uint64:{ fix_read_and_push_value<uint64_t>(L, buf); break; }
      case adata_et_int8:{ read_and_push_value<int8_t>(L, buf); break; }
      case adata_et_uint8:{ read_and_push_value<uint8_t>(L, buf); break; }
      case adata_et_int16:{ read_and_push_value<int16_t>(L, buf); break; }
      case adata_et_uint16:{ read_and_push_value<uint16_t>(L, buf); break; }
      case adata_et_int32:{ read_and_push_value<int32_t>(L, buf); break; }
      case adata_et_uint32:{ read_and_push_value<uint32_t>(L, buf); break; }
      case adata_et_int64:{ read_and_push_value<int64_t>(L, buf); break; }
      case adata_et_uint64:{ read_and_push_value<uint64_t>(L, buf); break; }
      case adata_et_float32:{ read_and_push_value<float>(L, buf); break; }
      case adata_et_float64:{ read_and_push_value<double>(L, buf); break; }
      case adata_et_string:
      {
        read_string(L, buf, size);
        break;
      }
      case adata_et_type:
      {
        if (type_define)
        {
          read_type(L, buf, type_define);
        }
        else
        {
          buf->set_error_code(undefined_member_protocol_not_compatible);
        }
        break;
      }
      default:
      {
        buf->set_error_code(undefined_member_protocol_not_compatible);
      }
      }
      if (buf->error())
      {
        return 0;
      }
      return 1;
    }

    static int read_member(lua_State *L, zero_copy_buffer * buf, adata_member * mb)
    {
      if (mb->type == adata_et_list)
      {
        uint32_t len = 0;
        adata::read(*buf, len);
        if (mb->size > 0 && (int)len > mb->size)
        {
          buf->set_error_code(number_of_element_not_macth);
          buf->trace_error(mb->name, -1);
          return 0;
        }
        lua_createtable(L, len, 0);
        adata_paramter_type * ptype = mb->paramter_type[0];
        for (uint32_t i = 1; i <= len; ++i)
        {
          read_value(L, buf, ptype->type, ptype->size, ptype->type_define);
          if (buf->error()) { buf->trace_error(mb->name, i - 1); return 0; }
          lua_rawseti(L, -2, i);
        }
      }
      else if (mb->type == adata_et_map)
      {
        uint32_t len = 0;
        adata::read(*buf, len);
        if (mb->size > 0 && (int)len > mb->size)
        {
          buf->set_error_code(number_of_element_not_macth);
          buf->trace_error(mb->name, -1);
          return 0;
        }
        adata_paramter_type * ptype1 = mb->paramter_type[0];
        adata_paramter_type * ptype2 = mb->paramter_type[1];
        lua_createtable(L, 0, len);
        for (uint32_t i = 0; i < len; ++i)
        {
          read_value(L, buf, ptype1->type, ptype1->size, ptype1->type_define);
          if (buf->error()) { buf->trace_error(mb->name, i + 1); return 0; }
          read_value(L, buf, ptype2->type, ptype2->size, ptype2->type_define);
          if (buf->error()) { buf->trace_error(mb->name, i + 1); return 0; }
          lua_rawset(L, -3);
        }
      }
      else
      {
        read_value(L, buf, mb->type, mb->size, mb->type_define);
        if (buf->error()) { buf->trace_error(mb->name, -1); return 0; }
      }
      return 1;
    }

    static int read_type(lua_State *L, zero_copy_buffer * buf, adata_type * type, bool create)
    {
      if (create)
      {
        lua_createtable(L, 0, (int)type->member_count);
        lua_rawgeti(L, 2, type->mt_idx);
        lua_setmetatable(L, -2);
      }
      uint64_t data_tag = 0;
      uint32_t data_len = 0;
      ::std::size_t offset = buf->read_length();
      read(*buf, data_tag);
      read(*buf, data_len);
      uint64_t mask = 1;
      for (size_t i = 0; i < type->member_count; ++i)
      {
        adata_member * mb = &type->members[i];
        int skip = 0;
        int read = 0;
        int create_default = 0;
        if (mb->del == 0)
        {
          if (data_tag&mask)
          {
            read = 1;
          }
          else
          {
            if (create)
            {
              create_default = 1;
            }
          }
        }
        else
        {
          if (data_tag&mask)
          {
            skip = 1;
          }
        }
        if (skip)
        {
          if (skip_read_member(L, buf, mb) == 0)
          {
            return 0;
          }
        }
        else if (read)
        {
          lua_rawgeti(L, 1, mb->field_idx);
          if (read_member(L, buf, mb) == 0)
          {
            lua_pop(L, 1);
            return 0;
          }
          else
          {
            lua_settable(L, -3);
          }
        }
        else if (create_default)
        {
          switch (mb->type)
          {
          case adata_et_string:
          {
            lua_rawgeti(L, 1, mb->field_idx);
            lua_pushlstring(L, "", 0);
            lua_settable(L, -3);
            break;
          }
          case adata_et_list:
          case adata_et_map:
          {
            lua_rawgeti(L, 1, mb->field_idx);
            lua_createtable(L, 0, 0);
            lua_settable(L, -3);
            break;
          }
          }
        }
        mask <<= 1;
      }
      ::std::size_t read_len = buf->read_length() - offset;
      ::std::size_t len = (::std::size_t)data_len;
      if (len > read_len) buf->skip_read(len - read_len);
      return 1;
    }

    static int lua_skip_read(lua_State * L)
    {
      zero_copy_buffer * zbuf = (zero_copy_buffer*)lua_touserdata(L, 3);
      adata_type * type = (adata_type*)lua_touserdata(L, 4);
      skip_read_type(L, zbuf, type);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    static int lua_read(lua_State * L)
    {
      zero_copy_buffer * zbuf = (zero_copy_buffer*)lua_touserdata(L, 3);
      adata_type * type = (adata_type*)lua_touserdata(L, 4);
      read_type(L, zbuf, type, false);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    struct type_sizeof_info
    {
      uint64_t tag;
      uint32_t size;
      type_sizeof_info() :tag(0), size(0){}
    };

    typedef std::vector<type_sizeof_info> type_sizeof_info_list_type;

    struct sizeof_cache_contex
    {
      type_sizeof_info_list_type list;
      size_t write_idx;
      sizeof_cache_contex() :write_idx(0){}
    };

    static int write_type(lua_State *L, zero_copy_buffer * buf, adata_type * type, sizeof_cache_contex& ctx);

    template<typename ty>
    ADATA_INLINE int lua_to_number(lua_State *L, int idx, ty& v)
    {
      v = (ty)lua_tointeger(L, idx);
      return 0;
    }

    template<>
    ADATA_INLINE int lua_to_number<float>(lua_State *L, int idx, float& v)
    {
      v = (float)lua_tonumber(L, idx);
      return 0;
    }

    template<>
    ADATA_INLINE int lua_to_number<double>(lua_State *L, int idx, double& v)
    {
      v = lua_tonumber(L, idx);
      return 0;
    }

#if LUA_VERSION_NUM < 503
    template<>
    ADATA_INLINE int lua_to_number<int64_t>(lua_State *L, int idx, int64_t& v)
    {
      number64_type it = { et_int64_unknow };
      lua_tonumber64(L, idx, &it);
      switch (it.type)
      {
      case et_int64_lua_number:{ v = (int64_t)it.value.d64; return 0; }
      case et_int64_int64:{ v = it.value.i64; return 0; }
      case et_int64_uint64:
      {
        if (it.value.u64 > (1ULL >> 53))
        {
          v = 0;
          return value_too_large_to_integer_number;
        }
        v = (int64_t)it.value.u64;
        return 0;
      }
      }
      return 0;
    }
#endif

    template<>
    ADATA_INLINE int lua_to_number<uint64_t>(lua_State *L, int idx, uint64_t& v)
    {
      number64_type it = { et_int64_unknow };
      lua_tonumber64(L, idx, &it);
      switch (it.type)
      {
      case et_int64_lua_number:
      {
        if (it.value.d64 < 0)
        {
          v = 0;
          return negative_assign_to_unsigned_integer_number;
        }
        v = (uint64_t)it.value.d64;
        return 0;
      }
      case et_int64_int64:
      {
        if (it.value.i64 < 0)
        {
          v = 0;
          return negative_assign_to_unsigned_integer_number;
        }
        v = (uint64_t)it.value.i64;
        return 0;
      }
      case et_int64_uint64:{ v = it.value.u64; }
      }
      return 0;
    }

    template<typename ty>
    void fix_pop_and_write_value(lua_State *L, zero_copy_buffer * buf)
    {
      ty v;
      int err = lua_to_number(L, -1, v);
      if (err)
      {
        buf->set_error_code((error_code_t)err);
      }
      adata::fix_write(*buf, v);
    }

    template<typename ty>
    void pop_and_write_value(lua_State *L, zero_copy_buffer * buf)
    {
      ty v;
      int err = lua_to_number(L, -1, v);
      if (err)
      {
        buf->set_error_code((error_code_t)err);
      }
      adata::write(*buf, v);
    }

    template<typename ty>
    int32_t fix_sizeof_value(lua_State *L)
    {
      ty v;
      lua_to_number(L, -1, v);
      return adata::fix_size_of(v);
    }

    template<typename ty>
    int32_t sizeof_value(lua_State *L)
    {
      ty v;
      lua_to_number(L, -1, v);
      return adata::size_of(v);
    }

    static ADATA_INLINE char * lua_get_string_ref(lua_State *L, int idx, size_t * slen)
    {
      int type = lua_type(L, idx);
      if (type != LUA_TSTRING)
      {
        slen = 0;
        return NULL;
      }
      size_t len = 0;
      char * str = (char *)lua_tolstring(L, idx, &len);
      if (slen != NULL)
      {
        *slen = len;
      }
      return str;
    }

    static ADATA_INLINE int32_t sizeof_string(lua_State *L)
    {
      size_t slen = 0;
      lua_get_string_ref(L, -1, &slen);
      int32_t str_len = (int32_t)slen;
      return str_len + adata::size_of(str_len);
    }

    static ADATA_INLINE int write_string(lua_State *L, zero_copy_buffer * buf, int sz)
    {
      size_t slen = 0;
      char * str = lua_get_string_ref(L, -1, &slen);
      if (sz > 0)
      {
        if (slen > (size_t)sz)
        {
          buf->set_error_code(number_of_element_not_macth);
          return 0;
        }
      }
      uint32_t str_len = (uint32_t)slen;
      adata::write(*buf, str_len);
      buf->write(str, slen);
      return 1;
    }

    static ADATA_INLINE int lua_get_len(lua_State *L, adata_member * mb)
    {
      int len = 0;
      switch (mb->type)
      {
      case adata_et_string:
      {
        if (lua_type(L, -1) != LUA_TSTRING)
        {
          return 0;
        }
        size_t slen = 0;
        lua_tolstring(L, -1,&slen);
        len = (int)slen;
        break;
      }
      case adata_et_list:
      {
        if (lua_type(L, -1) != LUA_TTABLE)
        {
          return 0;
        }
#if LUA_VERSION_NUM == 501
        len = (int)lua_objlen(L, -1);
#else
        len = (int)lua_rawlen(L, -1);
#endif
        break;
      }
      case adata_et_map:
      {
        if (lua_type(L, -1) != LUA_TTABLE)
        {
          return 0;
        }
        lua_pushnil(L);
        while (lua_next(L, -2) != 0)
        {
          lua_pop(L, 1);
          ++len;
        }
        break;
      }
      }
      return len;
    }

    static ADATA_INLINE bool test_adata_empty(lua_State *L, adata_member * mb)
    {
      switch (mb->type)
      {
      case adata_et_string:
      case adata_et_list:
      case adata_et_map:
      {
        return lua_get_len(L, mb) == 0;
      }
      }
      return false;
    }

    static int sizeof_type(lua_State *L, adata_type * type, sizeof_cache_contex * ctx = NULL);

    static ADATA_INLINE int32_t sizeof_value(lua_State *L, int type, int size, adata_type * type_define, sizeof_cache_contex * ctx)
    {
      (size);
      switch (type)
      {
      case adata_et_fix_int8:{ return fix_sizeof_value<int8_t>(L); }
      case adata_et_fix_uint8:{ return fix_sizeof_value<uint8_t>(L); }
      case adata_et_fix_int16:{ return fix_sizeof_value<int16_t>(L); }
      case adata_et_fix_uint16:{ return fix_sizeof_value<uint16_t>(L); }
      case adata_et_fix_int32:{ return fix_sizeof_value<int32_t>(L); }
      case adata_et_fix_uint32:{ return fix_sizeof_value<uint32_t>(L); }
      case adata_et_fix_int64:{ return fix_sizeof_value<int64_t>(L); }
      case adata_et_fix_uint64:{ return fix_sizeof_value<uint64_t>(L); }
      case adata_et_int8:{ return sizeof_value<int8_t>(L); }
      case adata_et_uint8:{ return sizeof_value<uint8_t>(L); }
      case adata_et_int16:{ return sizeof_value<int16_t>(L); }
      case adata_et_uint16:{ return sizeof_value<uint16_t>(L); }
      case adata_et_int32:{ return sizeof_value<int32_t>(L); }
      case adata_et_uint32:{ return sizeof_value<uint32_t>(L); }
      case adata_et_int64:{ return sizeof_value<int64_t>(L); }
      case adata_et_uint64:{ return sizeof_value<uint64_t>(L); }
      case adata_et_float32:{ return sizeof_value<float>(L); }
      case adata_et_float64:{ return sizeof_value<double>(L); }
      case adata_et_string:
      {
        return sizeof_string(L);
        break;
      }
      case adata_et_type:
      {
        if (type_define)
        {
          return sizeof_type(L, type_define, ctx);
        }
        break;
      }
      }
      return 0;
    }

    static int32_t sizeof_member(lua_State *L, adata_member * mb, sizeof_cache_contex * ctx)
    {
      int32_t size = 0;
      if (mb->type == adata_et_list)
      {
#if LUA_VERSION_NUM == 501
        int len = (int)lua_objlen(L, -1);
#else
        int len = (int)lua_rawlen(L, -1);
#endif
        size += adata::size_of(len);
        adata_paramter_type * ptype = mb->paramter_type[0];
        for (int i = 1; i <= len; ++i)
        {
          lua_rawgeti(L, -1, i);
          size += sizeof_value(L, ptype->type, ptype->size, ptype->type_define, ctx);
          lua_pop(L, 1);
        }
      }
      else if (mb->type == adata_et_map)
      {
        adata_paramter_type * ptype1 = mb->paramter_type[0];
        adata_paramter_type * ptype2 = mb->paramter_type[1];
        lua_pushnil(L);
        uint32_t i = 1;
        while (lua_next(L, -2))
        {
          lua_pushvalue(L, -2);
          size += sizeof_value(L, ptype1->type, ptype1->size, ptype1->type_define, ctx);
          lua_pop(L, 1);
          size += sizeof_value(L, ptype2->type, ptype2->size, ptype2->type_define, ctx);
          lua_pop(L, 1);
          ++i;
        }
        size += adata::size_of(--i);
      }
      else
      {
        size += sizeof_value(L, mb->type, mb->size, mb->type_define, ctx);
      }
      return size;
    }

    static int sizeof_type(lua_State *L, adata_type * type, sizeof_cache_contex * ctx)
    {
      type_sizeof_info info;
      size_t top = 0;
      if (ctx)
      {
        ctx->list.push_back(info);
        top = ctx->list.size() - 1;
      }
      uint64_t mask = 1;
      for (size_t i = 0; i < type->member_count; ++i)
      {
        adata_member * mb = &type->members[i];
        if (mb->del == 0)
        {
          lua_rawgeti(L, 1, mb->field_idx);
          lua_gettable(L, -2);
          if (test_adata_empty(L, mb) == false)
          {
            info.tag |= mask;
            info.size += sizeof_member(L, mb, ctx);
          }
          lua_pop(L, 1);
        }
        mask <<= 1;
      }
      info.size += adata::size_of(info.tag);
      info.size += adata::size_of(info.size);
      if (ctx)
      {
        ctx->list[top] = info;
      }
      return info.size;
    }

    static int lua_sizeof(lua_State * L)
    {
      adata_type * type = (adata_type*)lua_touserdata(L, 3);
      int size = sizeof_type(L, type, NULL);
      lua_pushinteger(L, size);
      return 1;
    }

    static ADATA_INLINE int write_value(lua_State *L, zero_copy_buffer * buf, int type, int size, adata_type * type_define, sizeof_cache_contex& ctx)
    {
      switch (type)
      {
      case adata_et_fix_int8:{ fix_pop_and_write_value<int8_t>(L, buf); break; }
      case adata_et_fix_uint8:{ fix_pop_and_write_value<uint8_t>(L, buf); break; }
      case adata_et_fix_int16:{ fix_pop_and_write_value<int16_t>(L, buf); break; }
      case adata_et_fix_uint16:{ fix_pop_and_write_value<uint16_t>(L, buf); break; }
      case adata_et_fix_int32:{ fix_pop_and_write_value<int32_t>(L, buf); break; }
      case adata_et_fix_uint32:{ fix_pop_and_write_value<uint32_t>(L, buf); break; }
      case adata_et_fix_int64:{ fix_pop_and_write_value<int64_t>(L, buf); break; }
      case adata_et_fix_uint64:{ fix_pop_and_write_value<uint64_t>(L, buf); break; }
      case adata_et_int8:{ pop_and_write_value<int8_t>(L, buf); break; }
      case adata_et_uint8:{ pop_and_write_value<uint8_t>(L, buf); break; }
      case adata_et_int16:{ pop_and_write_value<int16_t>(L, buf); break; }
      case adata_et_uint16:{ pop_and_write_value<uint16_t>(L, buf); break; }
      case adata_et_int32:{ pop_and_write_value<int32_t>(L, buf); break; }
      case adata_et_uint32:{ pop_and_write_value<uint32_t>(L, buf); break; }
      case adata_et_int64:{ pop_and_write_value<int64_t>(L, buf); break; }
      case adata_et_uint64:{ pop_and_write_value<uint64_t>(L, buf); break; }
      case adata_et_float32:{ pop_and_write_value<float>(L, buf); break; }
      case adata_et_float64:{ pop_and_write_value<double>(L, buf); break; }
      case adata_et_string:
      {
        write_string(L, buf, size);
        break;
      }
      case adata_et_type:
      {
        if (type_define)
        {
          write_type(L, buf, type_define, ctx);
        }
        else
        {
          buf->set_error_code(undefined_member_protocol_not_compatible);
        }
        break;
      }
      default:
      {
        buf->set_error_code(undefined_member_protocol_not_compatible);
      }
      }
      return 0;
    }

    static int lua_table_len(lua_State * L)
    {
      int c = 0;
      lua_pushnil(L);
      while (lua_next(L, -2) != 0)
      {
        lua_pop(L, 1);
        ++c;
      }
      return c;
    }

    static int write_member(lua_State *L, zero_copy_buffer * buf, adata_member * mb, sizeof_cache_contex& ctx)
    {
      if (mb->type == adata_et_list)
      {
#if LUA_VERSION_NUM == 501
        int len = (int)lua_objlen(L, -1);
#else
        int len = (int)lua_rawlen(L, -1);
#endif
        if (mb->size && len > mb->size)
        {
          buf->set_error_code(number_of_element_not_macth);
          buf->trace_error(mb->name, -1);
          return 0;
        }
        adata::write(*buf, len);
        adata_paramter_type * ptype = mb->paramter_type[0];
        for (int i = 1; i <= len; ++i)
        {
          lua_rawgeti(L, -1, i);
          write_value(L, buf, ptype->type, ptype->size, ptype->type_define, ctx);
          if (buf->error()) { buf->trace_error(mb->name, i); return 0; }
          lua_pop(L, 1);
        }
      }
      else if (mb->type == adata_et_map)
      {
        int len = lua_table_len(L);
        if (mb->size && len > mb->size)
        {
          buf->set_error_code(number_of_element_not_macth);
          buf->trace_error(mb->name, -1);
          return 0;
        }
        adata::write(*buf, len);
        adata_paramter_type * ptype1 = mb->paramter_type[0];
        adata_paramter_type * ptype2 = mb->paramter_type[1];
        lua_pushnil(L);
        uint32_t i = 1;
        while (lua_next(L, -2))
        {
          lua_pushvalue(L, -2);
          write_value(L, buf, ptype1->type, ptype1->size, ptype1->type_define, ctx);
          if (buf->error()) { buf->trace_error(mb->name, i); return 0; }
          lua_pop(L, 1);
          write_value(L, buf, ptype2->type, ptype2->size, ptype2->type_define, ctx);
          if (buf->error()) { buf->trace_error(mb->name, i); return 0; }
          lua_pop(L, 1);
          ++i;
        }
      }
      else
      {
        write_value(L, buf, mb->type, mb->size, mb->type_define, ctx);
        if (buf->error()) { buf->trace_error(mb->name, -1); return 0; }
      }
      return 1;
    }

    static int write_type(lua_State *L, zero_copy_buffer * buf, adata_type * type, sizeof_cache_contex& ctx)
    {
      type_sizeof_info& info = ctx.list[ctx.write_idx++];
      uint64_t data_tag = info.tag;
      int32_t  data_len = info.size;
      adata::write(*buf, data_tag);
      adata::write(*buf, data_len);
      uint64_t mask = 1;
      for (size_t i = 0; i < type->member_count; ++i)
      {
        if (data_tag&mask)
        {
          adata_member * mb = &type->members[i];
          lua_rawgeti(L, 1, mb->field_idx);
          lua_gettable(L, -2);
          if (write_member(L, buf, mb, ctx) == 0)
          {
            return 0;
          }
          lua_pop(L, 1);
        }
        mask <<= 1;
      }
      return 1;
    }

    static int lua_write(lua_State * L)
    {
      zero_copy_buffer * zbuf = (zero_copy_buffer*)lua_touserdata(L, 3);
      adata_type * type = (adata_type*)lua_touserdata(L, 4);
      sizeof_cache_contex ctx;
      sizeof_type(L, type, &ctx);
      write_type(L, zbuf, type, ctx);
      lua_pushinteger(L, zbuf->error_code());
      return 1;
    }

    ADATA_INLINE const luaL_Reg * build_lib()
    {
      static const luaL_Reg lib[] =
      {
        { "load", lua_load_pack },
        { "read", lua_read },
        { "skip_read", lua_skip_read },
        { "size_of", lua_sizeof },
        { "write", lua_write },
        { "new_buf", new_zbuf },
        { "del_buf", del_zbuf },
        { "resize_buf", resize_zuf },
        { "clear_buf", clear_zuf },
        { "set_error", set_error_zuf },
        { "trace_error", trace_error_zuf },
        { "trace_info", trace_info_zuf },
        { "get_rd_len", get_read_length },
        { "get_wt_len", get_write_length },
        { "get_write_data", get_write_buf_zuf },
        { "set_read_data", set_read_buf_zuf },
        { "rd_tag", read_tag },
        { "wt_tag", write_tag },
        { "rd_fixi8", read_fix_int8 },
        { "rd_fixu8", read_fix_uint8 },
        { "rd_fixi16", read_fix_int16 },
        { "rd_fixu16", read_fix_uint16 },
        { "rd_fixi32", read_fix_int32 },
        { "rd_fixu32", read_fix_uint32 },
        { "rd_fixi64", read_fix_int64 },
        { "rd_fixu64", read_fix_uint64 },
        { "rd_i8", read_int8 },
        { "rd_u8", read_uint8 },
        { "rd_i16", read_int16 },
        { "rd_u16", read_uint16 },
        { "rd_i32", read_int32 },
        { "rd_u32", read_uint32 },
        { "rd_i64", read_int64 },
        { "rd_u64", read_uint64 },
        { "rd_f32", read_float32 },
        { "rd_f64", read_float64 },
        { "rd_str", read_str },
        { "skip_rd_len", skip_read },
        { "skip_rd_fixi8", skip_read_fix_int8 },
        { "skip_rd_fixu8", skip_read_fix_uint8 },
        { "skip_rd_fixi16", skip_read_fix_int16 },
        { "skip_rd_fixu16", skip_read_fix_uint16 },
        { "skip_rd_fixi32", skip_read_fix_int32 },
        { "skip_rd_fixu32", skip_read_fix_uint32 },
        { "skip_rd_fixi64", skip_read_fix_int64 },
        { "skip_rd_fixu64", skip_read_fix_uint64 },
        { "skip_rd_i8", skip_read_int8 },
        { "skip_rd_u8", skip_read_uint8 },
        { "skip_rd_i16", skip_read_int16 },
        { "skip_rd_u16", skip_read_uint16 },
        { "skip_rd_i32", skip_read_int32 },
        { "skip_rd_u32", skip_read_uint32 },
        { "skip_rd_i64", skip_read_int64 },
        { "skip_rd_u64", skip_read_uint64 },
        { "skip_rd_f32", skip_read_float32 },
        { "skip_rd_f64", skip_read_float64 },
        { "skip_rd_str", skip_read_str },
        { "wt_fixi8", write_fix_int8 },
        { "wt_fixu8", write_fix_uint8 },
        { "wt_fixi16", write_fix_int16 },
        { "wt_fixu16", write_fix_uint16 },
        { "wt_fixi32", write_fix_int32 },
        { "wt_fixu32", write_fix_uint32 },
        { "wt_fixi64", write_fix_int64 },
        { "wt_fixu64", write_fix_uint64 },
        { "wt_i8", write_int8 },
        { "wt_u8", write_uint8 },
        { "wt_i16", write_int16 },
        { "wt_u16", write_uint16 },
        { "wt_i32", write_int32 },
        { "wt_u32", write_uint32 },
        { "wt_i64", write_int64 },
        { "wt_u64", write_uint64 },
        { "wt_f32", write_float32 },
        { "wt_f64", write_float64 },
        { "wt_str", write_str },
        { "szof_fixi8", size_of_fix_int8 },
        { "szof_fixu8", size_of_fix_uint8 },
        { "szof_fixi16", size_of_fix_int16 },
        { "szof_fixu16", size_of_fix_uint16 },
        { "szof_fixi32", size_of_fix_int32 },
        { "szof_fixu32", size_of_fix_uint32 },
        { "szof_fixi64", size_of_fix_int64 },
        { "szof_fixu64", size_of_fix_uint64 },
        { "szof_i8", size_of_int8 },
        { "szof_u8", size_of_uint8 },
        { "szof_i16", size_of_int16 },
        { "szof_u16", size_of_uint16 },
        { "szof_i32", size_of_int32 },
        { "szof_u32", size_of_uint32 },
        { "szof_i64", size_of_int64 },
        { "szof_u64", size_of_uint64 },
        { "szof_f32", size_of_float32 },
        { "szof_f64", size_of_float64 },
        { "szof_str", size_of_str },

        { NULL, NULL }
      };

      return lib;
    }

#if LUA_VERSION_NUM == 501
    ADATA_INLINE int init_adata_corec(lua_State *L)
    {
      init_lua_int64(L);

      static const luaL_Reg buf_meta_table[] =
      {
        { "__gc", del_zbuf },
        { "resize", resize_zuf },
        { "clear", clear_zuf },
        { "set_error", set_error_zuf },
        { "trace_error", trace_error_zuf },
        { "trace_info", trace_info_zuf },
        { "read_len", get_read_length },
        { "write_len", get_write_length },
        { "get_write_data", get_write_buf_zuf },
        { "set_read_data", set_read_buf_zuf },
        { NULL, NULL }
      };

      luaL_newmetatable(L, zbuffer_metatable);
      luaL_setfuncs(L, buf_meta_table, 0);
      lua_pushvalue(L, -1);
      lua_setfield(L, -2, "__index");

      const luaL_Reg * lib = build_lib();

      luaL_register(L, "adata_core", lib);
      return 1;
    }

#else

    ADATA_INLINE int regist_adata_core(lua_State *L)
    {
      luaL_checkversion(L);
      static const luaL_Reg buf_meta_table[] =
      {
        { "__gc", del_zbuf },
        { "resize", resize_zuf },
        { "clear", clear_zuf },
        { "set_error", set_error_zuf },
        { "trace_error", trace_error_zuf },
        { "trace_info", trace_info_zuf },
        { "read_len", get_read_length },
        { "write_len", get_write_length },
        { "get_write_data", get_write_buf_zuf },
        { "set_read_data", set_read_buf_zuf },
        { NULL, NULL }
      };

      luaL_newmetatable(L, zbuffer_metatable);
      luaL_setfuncs(L, buf_meta_table, 0);
      lua_pushvalue(L, -1);
      lua_setfield(L, -2, "__index");

      const luaL_Reg * lib = build_lib();
      luaL_newlib(L, lib);
      return 1;
    }

    ADATA_INLINE int init_adata_corec(lua_State *L)
    {
      init_lua_int64(L);
      luaL_requiref(L, "adata_core", regist_adata_core, 1);
      return 1;
    }

#endif

  }
}


#endif
