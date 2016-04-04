///
/// Copyright (c) 2009-2015GCE_ASIO_DETAIL_LUA_WRAP_HPP Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_DETAIL_LUA_WRAP_HPP
#define GCE_REDIS_DETAIL_LUA_WRAP_HPP

#include <gce/redis/config.hpp>
#include <gce/redis/conn.hpp>
#include <gce/redis/conn.adl.c2l.h>
#include <gce/redis/context_id.hpp>
#include <gce/redis/context_id.adl.c2l.h>
#include <gce/redis/context.hpp>
#include <gce/redis/session.hpp>
#include <gce/redis/errno.hpp>
#include <gce/redis/lua_object.hpp>
#include <gce/lualib/all.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <cstring>

namespace gce
{
namespace redis
{
namespace detail
{
namespace lua
{
///------------------------------------------------------------------------------
/// context_id
///------------------------------------------------------------------------------
struct context_id
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static void create(lua_State* L, ctxid_t const& ctxid = make_ctxid())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, ctxid, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, ctxid);
#endif
  }
};

static void load(lua_State* L, int arg, ctxid_t& ctxid)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, ctxid);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}

static void push(lua_State* L, ctxid_t const& ctxid)
{
  context_id::create(L, ctxid);
}
///------------------------------------------------------------------------------
/// conn
///------------------------------------------------------------------------------
struct conn
{
  typedef boost::asio::ip::tcp::resolver resolver_t;
  typedef gce::asio::detail::lua::tcp_endpoint_itr endpoint_t;
  static int make(lua_State* L)
  {
    try
    {
      if (gce::lua::is_argnil(L, 1))
      {
        create(L);
        return 1;
      }

      conn_t c;
      redis::ctxid_t ctxid;
      load(L, 1, ctxid);

      int ty2 = lua_type(L, 2);
      luaL_argcheck(
        L, (ty2 == LUA_TUSERDATA || ty2 == LUA_TTABLE), 
        2, "'table' or 'userdata' expected"
        );

      gce::duration_t timeout = gce::seconds(15);
      if (!gce::lua::is_argnil(L, 3))
      {
        gce::lua::load(L, 3, timeout);
      }

      if (ty2 == LUA_TTABLE)
      {
        /// {host, port}
        lua_rawgeti(L, 2, 1);
        size_t len = 0;
        char const* host = luaL_checklstring(L, -1, &len);
        lua_pop(L, 1);
        lua_rawgeti(L, 2, 2);
        lua_Integer port = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        c = gce::redis::make_conn(ctxid, std::string(host, len), (uint16_t)port, timeout);
      }
      else
      {
        /// tcp_endpoint_itr
        resolver_t::iterator eitr = resolver_t::iterator();
        endpoint_t* itr = gce::lua::from_lua<endpoint_t>(L, 2, "tcp_endpoint_itr");
        eitr = *itr->object()->get();
        c = gce::redis::make_conn(ctxid, eitr, timeout);
      }

      create(L, c);
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static void create(lua_State* L, conn_t const& c = conn_t())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, c, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, c);
#endif
  }
};

static void load(lua_State* L, int arg, conn_t& c)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, c);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}

static void push(lua_State* L, conn_t const& c)
{
  conn::create(L, c);
}
///------------------------------------------------------------------------------
/// array_ref
///------------------------------------------------------------------------------
struct array_ref
{
  gce::packer& pkr_;
  resp::unique_array<resp::unique_value> const& arr_;

  explicit array_ref(
    gce::packer& pkr, 
    resp::unique_array<resp::unique_value> const& arr
    )
    : pkr_(pkr)
    , arr_(arr)
  {
  }

  ~array_ref()
  {
  }

  static char const* name()
  {
    return "array_ref";
  }

  static array_ref* create(
    lua_State* L, 
    gce::packer& pkr, 
    resp::unique_array<resp::unique_value> const& arr
    )
  {
    void* block = lua_newuserdata(L, sizeof(array_ref));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for array_ref failed");
      return 0;
    }

    array_ref* o = new (block) array_ref(pkr, arr);
    gce::lualib::setmetatab(L, "libredis", name());
    return o;
  }

  static int gc(lua_State* L)
  {
    array_ref* wrap = static_cast<array_ref*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~array_ref();
    }
    return 0;
  }

  static int type(lua_State* L)
  {
    array_ref* o = gce::lua::from_lua<array_ref>(L, 1, name());
    size_t index = (size_t)luaL_checkinteger(L, 2);
    lua_Integer ty = (lua_Integer)o->arr_[index].type();
    lua_pushinteger(L, ty);
    return 1;
  }

  static int integer(lua_State* L)
  {
    array_ref* o = gce::lua::from_lua<array_ref>(L, 1, name());
    size_t index = (size_t)luaL_checkinteger(L, 2);
    int64_t i = o->arr_[index].integer();
    lua_pushnumber(L, (lua_Number)i);
    return 1;
  }

  static int string(lua_State* L)
  {
    array_ref* o = gce::lua::from_lua<array_ref>(L, 1, name());
    size_t index = (size_t)luaL_checkinteger(L, 2);
    resp::buffer const& str = o->arr_[index].string();
    lua_pushlstring(L, str.data(), str.size());
    return 1;
  }

  static int error(lua_State* L)
  {
    array_ref* o = gce::lua::from_lua<array_ref>(L, 1, name());
    size_t index = (size_t)luaL_checkinteger(L, 2);
    resp::buffer const& str = o->arr_[index].error();
    lua_pushlstring(L, str.data(), str.size());
    return 1;
  }

  static int bulkstr(lua_State* L)
  {
    array_ref* o = gce::lua::from_lua<array_ref>(L, 1, name());
    size_t index = (size_t)luaL_checkinteger(L, 2);
    resp::buffer const& str = o->arr_[index].bulkstr();
    lua_pushlstring(L, str.data(), str.size());
    return 1;
  }

  static int array(lua_State* L)
  {
    array_ref* o = gce::lua::from_lua<array_ref>(L, 1, name());
    size_t index = (size_t)luaL_checkinteger(L, 2);
    resp::unique_array<resp::unique_value> const& arr = o->arr_[index].array();
    create(L, o->pkr_, arr);
    return 1;
  }

  static int get(lua_State* L)
  {
    array_ref* o = gce::lua::from_lua<array_ref>(L, 1, name());
    size_t index = (size_t)luaL_checkinteger(L, 2);
    resp::buffer const& bin = o->arr_[index].bulkstr();
    o->pkr_.set_read((byte_t const*)bin.data(), bin.size());
    gce::lua::unpack_object(L, o->pkr_, 3);
    return 1;
  }
};
///------------------------------------------------------------------------------
/// result
///------------------------------------------------------------------------------
struct result
  : public gce::lua::basic_object
{
  typedef redis::result_ptr object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "result";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_;
  }

  virtual void unpack(gce::message& msg)
  {
    msg >> obj_;
  }

  virtual void pack(gce::packer& pkr)
  {
    GCE_ASSERT(false);
  }

  virtual void unpack(gce::packer& pkr)
  {
    GCE_ASSERT(false);
  }

  virtual int gcety() const
  {
    return gce::redis::lua::ty_result;
  }

  static int make(lua_State* L)
  {
    void* block = lua_newuserdata(L, sizeof(result));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for result failed");
    }

    new (block) result;
    gce::lualib::setmetatab(L, "libredis", name());
    return 1;
  }

  static int type(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    resp::value_type type = o->type();
    lua_Integer ty = (lua_Integer)type;
    lua_pushinteger(L, ty);
    return 1;
  }

  static int integer(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    int64_t i = o->integer();
    lua_pushnumber(L, (lua_Number)i);
    return 1;
  }

  static int string(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    resp::buffer const& str = o->string();
    lua_pushlstring(L, str.data(), str.size());
    return 1;
  }

  static int error(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    resp::buffer const& str = o->error();
    lua_pushlstring(L, str.data(), str.size());
    return 1;
  }

  static int bulkstr(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    resp::buffer const& str = o->bulkstr();
    lua_pushlstring(L, str.data(), str.size());
    return 1;
  }

  static int array(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    resp::unique_array<resp::unique_value> const& arr = o->array();
    array_ref::create(L, o->pkr_, arr);
    return 1;
  }

  static int get(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    resp::buffer const& bin = o->bulkstr();
    o->pkr_.set_read((byte_t const*)bin.data(), bin.size());
    gce::lua::unpack_object(L, o->pkr_, 2);
    return 1;
  }

  static int gc(lua_State* L)
  {
    result* wrap = static_cast<result*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~result();
    }
    return 0;
  }

  static int gcety(lua_State* L)
  {
    result* wrap = gce::lua::from_lua<result>(L, 1, name());
    lua_pushinteger(L, wrap->gcety());
    return 1;
  }
};
///------------------------------------------------------------------------------
/// session
///------------------------------------------------------------------------------
struct session
{
  static char const* name()
  {
    return "session";
  }

  static int make(lua_State* L)
  {
    try
    {
      gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
      conn_t c;
      load(L, 2, c);

      snid_t snid = snid_nil;
      if (!gce::lua::is_argnil(L, 3))
      {
        gce::lua::load(L, 3, snid);
      }

      void* block = lua_newuserdata(L, sizeof(redis::session));
      GCE_VERIFY(block != 0).msg("lua_newuserdata for redis::session failed");

      redis::session* o = new (block) redis::session(*a, c, snid);

      lua_pushvalue(L, -1);
      int k = gce::lualib::make_ref(L, "libredis");
      (*a)->add_addon("libredis", k, o);
      gce::lualib::setmetatab(L, "libredis", name());
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static int open(lua_State* L)
  {
    redis::session* o = gce::lua::from_lua<redis::session>(L, 1, name());
    o->open();
    return 0;
  }

  static int cmd(lua_State* L)
  {
    redis::session* o = gce::lua::from_lua<redis::session>(L, 1, name());
    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);
    redis::session::luactx luactx = o->get_luactx();

    int argn = 3;
    boost::string_ref cmd_name(str, len);
    if (gce::lua::is_argnil(L, argn + 0))
    {
      o->cmd(cmd_name);
    }
    else if (gce::lua::is_argnil(L, argn + 1))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
    }
    else if (gce::lua::is_argnil(L, argn + 2))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
    }
    else if (gce::lua::is_argnil(L, argn + 3))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
    }
    else if (gce::lua::is_argnil(L, argn + 4))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
      arg(L, o, luactx, argn + 3);
    }
    else if (gce::lua::is_argnil(L, argn + 5))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
      arg(L, o, luactx, argn + 3);
      arg(L, o, luactx, argn + 4);
    }
    else
    {
      return luaL_error(L, "arg num can't over 5");
    }

    /// return session itself
    lua_pushvalue(L, 1);
    return 1;
  }

  static int args(lua_State* L)
  {
    redis::session* o = gce::lua::from_lua<redis::session>(L, 1, name());
    redis::session::luactx luactx = o->get_luactx();

    int argn = 2;
    if (gce::lua::is_argnil(L, argn + 0))
    {
    }
    else if (gce::lua::is_argnil(L, argn + 1))
    {
      arg(L, o, luactx, argn + 0);
    }
    else if (gce::lua::is_argnil(L, argn + 2))
    {
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
    }
    else if (gce::lua::is_argnil(L, argn + 3))
    {
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
    }
    else if (gce::lua::is_argnil(L, argn + 4))
    {
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
      arg(L, o, luactx, argn + 3);
    }
    else if (gce::lua::is_argnil(L, argn + 5))
    {
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
      arg(L, o, luactx, argn + 3);
      arg(L, o, luactx, argn + 4);
    }
    else
    {
      return luaL_error(L, "arg num can't over 5");
    }

    /// return session itself
    lua_pushvalue(L, 1);
    return 1;
  }

private:
  static void arg(
    lua_State* L, 
    redis::session* o, 
    redis::session::luactx& luactx, 
    int objidx
    )
  {
    resp::encoder<resp::buffer>::command& cmd = luactx.cmd_;
    message& large_args_buffer = luactx.curr_req_->large_args_buffer_;

    int ty = lua_type(L, objidx);
    switch (ty)
    {
    case LUA_TNUMBER: 
    {
      lua_Number num = luaL_checknumber(L, objidx);
      o->args(num);
    }break;
    case LUA_TBOOLEAN:
    {
      int b = lua_toboolean(L, objidx);
      o->args(b);
    }break;
    case LUA_TSTRING: 
    {
      char const* str = luaL_checkstring(L, objidx);
      o->args(str);
    }break;
    case LUA_TTABLE:
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
    {
      large_args_buffer.to_large(RESP_LARGE_BUFFER_SIZE);
      byte_t const* begin_buf = large_args_buffer.reset_write(0);
      //large_args_buffer << t;
      gce::lua::pack_object(L, large_args_buffer, objidx);
      byte_t const* end_buf = large_args_buffer.reset_write(0);
      cmd.arg(resp::buffer((char const*)begin_buf, end_buf - begin_buf));
    }break;
    default:
    {
      luaL_error(L, "metatable not found!");
    }break;
    }
  }

public:
  static int execute(lua_State* L)
  {
    redis::session* o = gce::lua::from_lua<redis::session>(L, 1, name());
    o->execute();
    return 0;
  }

  static int query(lua_State* L)
  {
    redis::session* o = gce::lua::from_lua<redis::session>(L, 1, name());
    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);
    redis::session::luactx luactx = o->get_luactx();

    int argn = 3;
    boost::string_ref cmd_name(str, len);
    if (gce::lua::is_argnil(L, argn + 0))
    {
      o->cmd(cmd_name);
      o->execute();
    }
    else if (gce::lua::is_argnil(L, argn + 1))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      o->execute();
    }
    else if (gce::lua::is_argnil(L, argn + 2))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      o->execute();
    }
    else if (gce::lua::is_argnil(L, argn + 3))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
      o->execute();
    }
    else if (gce::lua::is_argnil(L, argn + 4))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
      arg(L, o, luactx, argn + 3);
      o->execute();
    }
    else if (gce::lua::is_argnil(L, argn + 5))
    {
      o->cmd(cmd_name);
      arg(L, o, luactx, argn + 0);
      arg(L, o, luactx, argn + 1);
      arg(L, o, luactx, argn + 2);
      arg(L, o, luactx, argn + 3);
      arg(L, o, luactx, argn + 4);
      o->execute();
    }
    else
    {
      return luaL_error(L, "arg num can't over 5");
    }
    return 0;
  }

  static int ping(lua_State* L)
  {
    redis::session* o = gce::lua::from_lua<redis::session>(L, 1, name());
    o->ping();
    return 0;
  }

  static int get_snid(lua_State* L)
  {
    redis::session* o = gce::lua::from_lua<redis::session>(L, 1, name());
    gce::redis::snid_t const& snid = o->get_snid();
    gce::lua::push(L, snid);
    return 1;
  }

  static int gc(lua_State* L)
  {
    redis::session* o = static_cast<redis::session*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~session();
    }
    return 0;
  }
};
///------------------------------------------------------------------------------
/// init_nil
///------------------------------------------------------------------------------
inline int init_nil(lua_State* L)
{
  /// set nil
  lua_getglobal(L, "libredis");
  gce::lua::push(L, redis::snid_nil);
  lua_setfield(L, -2, "snid_nil");
  gce::lua::push(L, redis::errno_nil);
  lua_setfield(L, -2, "errno_nil");
  lua_pop(L, 1);
  return 0;
}
///------------------------------------------------------------------------------
} /// namespace lua
} /// namespace detail
} /// namespace redis
} /// namespace gce

#endif /// GCE_REDIS_DETAIL_LUA_WRAP_HPP
