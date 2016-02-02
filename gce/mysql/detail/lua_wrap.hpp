///
/// Copyright (c) 2009-2015GCE_ASIO_DETAIL_LUA_WRAP_HPP Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_DETAIL_LUA_WRAP_HPP
#define GCE_MYSQL_DETAIL_LUA_WRAP_HPP

#include <gce/mysql/config.hpp>
#include <gce/mysql/context.hpp>
#include <gce/mysql/fetcher.hpp>
#include <gce/mysql/conn_option.hpp>
#include <gce/mysql/conn_option.adl.c2l.h>
#include <gce/mysql/conn.hpp>
#include <gce/mysql/errno.hpp>
#include <gce/mysql/conn.adl.c2l.h>
#include <gce/mysql/context_id.hpp>
#include <gce/mysql/context_id.adl.c2l.h>
#include <gce/mysql/session.hpp>
#include <gce/mysql/lua_object.hpp>
#include <gce/lualib/all.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/polymorphic_cast.hpp>
#include <boost/array.hpp>
#include <string>
#include <cstring>

namespace gce
{
namespace mysql
{
namespace detail
{
namespace lua
{
///------------------------------------------------------------------------------
/// conn_option
///------------------------------------------------------------------------------
struct conn_option
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static void create(lua_State* L, connopt_t const& opt = make_connopt())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, opt, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, opt);
#endif
  }
};

static void load(lua_State* L, int arg, connopt_t& opt)
{
  if (arg != -1)
  {
    lua_pushvalue(L, arg);
  }
  adata::lua::load(L, opt);
  if (arg != -1)
  {
    lua_pop(L, 1);
  }
}

static void push(lua_State* L, connopt_t const& opt)
{
  conn_option::create(L, opt);
}
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
  static int make(lua_State* L)
  {
    try
    {
      if (gce::lua::is_argnil(L, 1))
      {
        create(L);
        return 1;
      }

      mysql::ctxid_t ctxid;
      load(L, 1, ctxid);

      char const* host = luaL_checkstring(L, 2);
      lua_Integer port = luaL_checkinteger(L, 3);
      char const* usr = luaL_checkstring(L, 4);
      size_t pwd_len = 0;
      char const* pwd = luaL_checklstring(L, 5, &pwd_len);
      char const* db = luaL_checkstring(L, 6);

      connopt_t opt = make_connopt();
      if (!gce::lua::is_argnil(L, 7))
      {
        load(L, 7, opt);
      }

      conn_t c = mysql::make_conn(ctxid, host, (uint16_t)port, usr, std::string(pwd, pwd_len), db, opt);
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
/// result
///------------------------------------------------------------------------------
struct result
  : public gce::lua::basic_object
{
  typedef mysql::result_ptr object_t;
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

  virtual int gcety() const
  {
    return gce::mysql::lua::ty_result;
  }

  static int make(lua_State* L)
  {
    void* block = lua_newuserdata(L, sizeof(result));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for result failed");
    }

    new (block) result;
    gce::lualib::setmetatab(L, "libmysql", name());
    return 1;
  }

  static int table_size(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    mysql::fetcher fch(o);
    lua_pushinteger(L, (lua_Integer)fch.table_size());
    return 1;
  }

  static int row_size(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    size_t i = (size_t)luaL_checkinteger(L, 2) - 1;
    mysql::fetcher fch(o);
    lua_pushinteger(L, (lua_Integer)fch.row_size(i));
    return 1;
  }

  static int field_size(lua_State* L)
  {
    object_t& o = *gce::lua::from_lua<result>(L, 1);
    if (!o)
    {
      return luaL_error(L, "result is empty!");
    }

    size_t i = (size_t)luaL_checkinteger(L, 2) - 1;
    mysql::fetcher fch(o);
    lua_pushinteger(L, (lua_Integer)fch.field_size(i));
    return 1;
  }

  static void pushvalue(lua_State* L, char const* row, long int len)
  {
    if (row == NULL)
    {
      lua_pushnil(L);
    }
    else
    {
      lua_pushlstring(L, row, len);
    }
  }

  static int fetch(lua_State* L)
  {
    try
    {
      object_t& o = *gce::lua::from_lua<result>(L, 1);
      GCE_VERIFY(o).msg("result is empty, fetch failed!");

      size_t tabidx = (size_t)luaL_checkinteger(L, 2) - 1;
      size_t rowidx = (size_t)luaL_checkinteger(L, 3) - 1;

      mysql::fetcher fch(o);
      mysql::row row = fch.get_row(tabidx, rowidx);

      int field_size = (int)row.field_size();
      int ty4 = lua_type(L, 4);
      if (gce::lua::is_argnil(L, 4, ty4))
      {
        /// fetch args directly
        for (int i=0; i<field_size; ++i)
        {
          errcode_t ec;
          boost::string_ref field = row.fetch_raw(i, ec);
          GCE_ASSERT(!ec);
          pushvalue(L, field.data(), field.size());
        }
        return field_size;
      }
      else
      {
        /// fetch args into table
        GCE_VERIFY(lua_istable(L, 4));
        char const* opts = luaL_optstring(L, 5, "n");
        if (std::strchr(opts, 'n') != NULL)
        {
          for (int i=0; i<field_size; ++i) 
          {
            errcode_t ec;
            boost::string_ref fstr = row.fetch_raw(i, ec);
            GCE_ASSERT(!ec);
            pushvalue(L, fstr.data(), fstr.size());
            lua_rawseti(L, 4, i+1);
          }
        }
        else if (std::strchr(opts, 'a') != NULL)
        {
          for (int i=0; i<field_size; ++i)
          {
            MYSQL_FIELD& field = row.get_field(i);
            errcode_t ec;
            boost::string_ref fstr = row.fetch_raw(i, ec);
            GCE_ASSERT(!ec);

            lua_pushstring(L, field.name);
            pushvalue(L, fstr.data(), fstr.size());
            lua_rawset(L, 4);
          }
        }
        lua_pushvalue(L, 4);
        return 1;
      }
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
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

      void* block = lua_newuserdata(L, sizeof(mysql::session));
      GCE_VERIFY(block != 0).msg("lua_newuserdata for mysql::session failed");

      mysql::session* o = new (block) mysql::session(*a, c, snid);

      lua_pushvalue(L, -1);
      int k = gce::lualib::make_ref(L, "libmysql");
      (*a)->add_addon("libmysql", k, o);
      gce::lualib::setmetatab(L, "libmysql", name());
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static int open(lua_State* L)
  {
    mysql::session* o = gce::lua::from_lua<mysql::session>(L, 1, name());
    o->open();
    return 0;
  }

  static int sql(lua_State* L)
  {
    mysql::session* o = gce::lua::from_lua<mysql::session>(L, 1, name());
    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);
    mysql::session::luactx luactx = o->get_luactx();

    int argn = 3;
    boost::string_ref sqlstr(str, len);
    if (gce::lua::is_argnil(L, argn + 0))
    {
      o->sql(sqlstr);
    }
    else if (gce::lua::is_argnil(L, argn + 1))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      o->sql(sqlstr, arg1);
    }
    else if (gce::lua::is_argnil(L, argn + 2))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      o->sql(sqlstr, arg1, arg2);
    }
    else if (gce::lua::is_argnil(L, argn + 3))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      o->sql(sqlstr, arg1, arg2, arg3);
    }
    else if (gce::lua::is_argnil(L, argn + 4))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      char const* arg4 = arg(L, luactx, argn + 3);
      o->sql(sqlstr, arg1, arg2, arg3, arg4);
    }
    else if (gce::lua::is_argnil(L, argn + 5))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      char const* arg4 = arg(L, luactx, argn + 3);
      char const* arg5 = arg(L, luactx, argn + 4);
      o->sql(sqlstr, arg1, arg2, arg3, arg4, arg5);
    }
    else if (gce::lua::is_argnil(L, argn + 6))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      char const* arg4 = arg(L, luactx, argn + 3);
      char const* arg5 = arg(L, luactx, argn + 4);
      char const* arg6 = arg(L, luactx, argn + 5);
      o->sql(sqlstr, arg1, arg2, arg3, arg4, arg5, arg6);
    }
    else if (gce::lua::is_argnil(L, argn + 7))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      char const* arg4 = arg(L, luactx, argn + 3);
      char const* arg5 = arg(L, luactx, argn + 4);
      char const* arg6 = arg(L, luactx, argn + 5);
      char const* arg7 = arg(L, luactx, argn + 6);
      o->sql(sqlstr, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    }
    else if (gce::lua::is_argnil(L, argn + 8))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      char const* arg4 = arg(L, luactx, argn + 3);
      char const* arg5 = arg(L, luactx, argn + 4);
      char const* arg6 = arg(L, luactx, argn + 5);
      char const* arg7 = arg(L, luactx, argn + 6);
      char const* arg8 = arg(L, luactx, argn + 7);
      o->sql(sqlstr, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
    }
    else if (gce::lua::is_argnil(L, argn + 9))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      char const* arg4 = arg(L, luactx, argn + 3);
      char const* arg5 = arg(L, luactx, argn + 4);
      char const* arg6 = arg(L, luactx, argn + 5);
      char const* arg7 = arg(L, luactx, argn + 6);
      char const* arg8 = arg(L, luactx, argn + 7);
      char const* arg9 = arg(L, luactx, argn + 8);
      o->sql(sqlstr, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
    }
    else if (gce::lua::is_argnil(L, argn + 10))
    {
      char const* arg1 = arg(L, luactx, argn + 0);
      char const* arg2 = arg(L, luactx, argn + 1);
      char const* arg3 = arg(L, luactx, argn + 2);
      char const* arg4 = arg(L, luactx, argn + 3);
      char const* arg5 = arg(L, luactx, argn + 4);
      char const* arg6 = arg(L, luactx, argn + 5);
      char const* arg7 = arg(L, luactx, argn + 6);
      char const* arg8 = arg(L, luactx, argn + 7);
      char const* arg9 = arg(L, luactx, argn + 8);
      char const* arg10 = arg(L, luactx, argn + 9);
      o->sql(sqlstr, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    }
    else
    {
      return luaL_error(L, "arg num can't over 10");
    }

    /// return session itself
    lua_pushvalue(L, 1);
    return 1;
  }

private:
  static void pri_sql(lua_State* L, mysql::session* o)
  {
  }

  static char const* arg(lua_State* L, mysql::session::luactx& luactx, int objidx)
  {
    std::deque<gce::intbuf_t>& tmp_num_list = luactx.tmp_num_list_;
    std::deque<gce::detail::buffer_st>& tmp_escape_string_list = luactx.tmp_escape_string_list_;
    gce::message& tmp_msg = luactx.tmp_msg_;
    int ty = lua_type(L, objidx);
    switch (ty)
    {
    case LUA_TNUMBER: 
    {
      lua_Number num = luaL_checknumber(L, objidx);
      tmp_num_list.push_back(boost::lexical_cast<intbuf_t>(num));
      return tmp_num_list.back().cbegin();
    }break;
    case LUA_TBOOLEAN:
    {
      int b = lua_toboolean(L, objidx);
      tmp_num_list.push_back(boost::lexical_cast<intbuf_t>(b));
      return tmp_num_list.back().cbegin();
    }break;
    case LUA_TSTRING: 
    {
      return luaL_checkstring(L, objidx);
    }break;
    case LUA_TTABLE:
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
    {
      tmp_msg.clear();
      gce::lua::pack_object(L, tmp_msg, objidx);
      tmp_escape_string_list.push_back(gce::detail::buffer_st());
      gce::detail::buffer_st& buf = tmp_escape_string_list.back();
      buf.resize(2 * tmp_msg.size() + 1);
      mysql_real_escape_string(
        luactx.mysql_, 
        (char*)buf.data(), 
        (char const*)tmp_msg.data(), tmp_msg.size()
        );
      return (char const*)buf.data();
    }break;
    default:
    {
      luaL_error(L, "metatable not found!");
    }break;
    }
    return 0;
  }

public:
  static int execute(lua_State* L)
  {
    mysql::session* o = gce::lua::from_lua<mysql::session>(L, 1, name());
    if (gce::lua::is_argnil(L, 2) || (!gce::lua::is_argnil(L, 2) && lua_type(L, 2) != LUA_TSTRING))
    {
      mysql::result_ptr res_ptr;
      if (!gce::lua::is_argnil(L, 2))
      {
        result::object_t* res = gce::lua::from_lua<result>(L, 2);
        if (!*res)
        {
          *res = boost::make_shared<mysql::result>();
        }
        res_ptr = *res;
      }
      o->execute(res_ptr);
      return 0;
    }

    size_t len = 0;
    char const* str = luaL_checklstring(L, 2, &len);

    mysql::result_ptr res_ptr;
    if (!gce::lua::is_argnil(L, 3))
    {
      result::object_t* res = gce::lua::from_lua<result>(L, 3);
      if (!*res)
      {
        *res = boost::make_shared<mysql::result>();
      }
      res_ptr = *res;
    }
    o->execute(boost::string_ref(str, len), res_ptr);
    return 0;
  }

  static int ping(lua_State* L)
  {
    mysql::session* o = gce::lua::from_lua<mysql::session>(L, 1, name());
    o->ping();
    return 0;
  }

  static int gc(lua_State* L)
  {
    mysql::session* o = static_cast<mysql::session*>(lua_touserdata(L, 1));
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
  lua_getglobal(L, "libmysql");
  gce::lua::push(L, mysql::snid_nil);
  lua_setfield(L, -2, "snid_nil");
  gce::lua::push(L, mysql::errno_nil);
  lua_setfield(L, -2, "errno_nil");
  lua_pop(L, 1);
  return 0;
}
///------------------------------------------------------------------------------
} /// namespace lua
} /// namespace detail
} /// namespace mysql
} /// namespace gce

#endif /// GCE_MYSQL_DETAIL_LUA_WRAP_HPP
