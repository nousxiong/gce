///
/// Copyright (c) 2009-2015GCE_ASIO_DETAIL_LUA_WRAP_HPP Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_ASIO_DETAIL_LUA_WRAP_HPP
#define GCE_ASIO_DETAIL_LUA_WRAP_HPP

#include <gce/asio/config.hpp>
#include <gce/asio/timer.hpp>
#include <gce/asio/signal.hpp>
#include <gce/asio/tcp/acceptor.hpp>
#include <gce/asio/tcp/socket.hpp>
#include <gce/asio/ssl/stream.hpp>
#include <gce/asio/tcp/option.hpp>
#include <gce/asio/tcp/tcp_option.adl.c2l.h>
#include <gce/asio/ssl/option.hpp>
#include <gce/asio/ssl/ssl_option.adl.c2l.h>
#include <gce/asio/lua_object.hpp>
#include <gce/lualib/all.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

namespace gce
{
namespace asio
{
namespace detail
{
namespace lua
{
///------------------------------------------------------------------------------
/// timer
///------------------------------------------------------------------------------
template <typename Impl>
struct timer
{
  typedef Impl impl_t;
  
  static std::string type()
  {
    std::string meta = "timer";
    meta += boost::lexical_cast<intbuf_t>(impl_t::type()).cbegin();
    return meta;
  }
  
  static int make(lua_State* L)
  {
    gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");

    void* block = lua_newuserdata(L, sizeof(impl_t));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for timer failed");
    }

    impl_t* o = 0;
    if (lua_type(L, 2) != LUA_TNIL)
    {
      gce::duration_t dur;
      gce::lua::load(L, 2, dur);
      o = new (block) impl_t(*a, dur);
    }
    else
    {
      o = new (block) impl_t(*a);
    }

    lua_pushvalue(L, -1);
    int k = gce::lualib::make_ref(L, "libasio");
    (*a)->add_addon("libasio", k, o);
    gce::lualib::setmetatab(L, type().c_str());
    return 1;
  }

  static int gc(lua_State* L)
  {
    impl_t* o = static_cast<impl_t*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~impl_t();
    }
    return 0;
  }

  static int async_wait(lua_State* L)
  {
    impl_t* o = gce::lua::from_lua<impl_t>(L, 1, "timer");

    gce::duration_t dur;
    gce::lua::load(L, 2, dur);

    if (lua_type(L, 3) != LUA_TNIL)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_wait(dur, *msg);
    }
    else
    {
      o->async_wait(dur);
    }
    return 0;
  }

  static int cancel(lua_State* L)
  {
    impl_t* o = gce::lua::from_lua<impl_t>(L, 1, "timer");
    
    errcode_t ec;
    (*o)->cancel(ec);
    
    lua_pushinteger(L, ec.value());
    return 1;
  }
};
///------------------------------------------------------------------------------
/// signal
///------------------------------------------------------------------------------
struct signal
{
  static int make(lua_State* L)
  {
    gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
    
    void* block = lua_newuserdata(L, sizeof(asio::signal));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for signal failed");
    }

    asio::signal* o = 0;
    if (lua_type(L, 4) == LUA_TNUMBER)
    {
      int sig1 = lua_tointeger(L, 2);
      int sig2 = lua_tointeger(L, 3);
      int sig3 = lua_tointeger(L, 4);
      o = new (block) asio::signal(*a, sig1, sig2, sig3);
    }
    else if (lua_type(L, 3) == LUA_TNUMBER)
    {
      int sig1 = lua_tointeger(L, 2);
      int sig2 = lua_tointeger(L, 3);
      o = new (block) asio::signal(*a, sig1, sig2);
    }
    else if (lua_type(L, 2) == LUA_TNUMBER)
    {
      int sig1 = lua_tointeger(L, 2);
      o = new (block) asio::signal(*a, sig1);
    }
    else
    {
      o = new (block) asio::signal(*a);
    }

    lua_pushvalue(L, -1);
    int k = gce::lualib::make_ref(L, "libasio");
    (*a)->add_addon("libasio", k, o);
    gce::lualib::setmetatab(L, "signal");
    return 1;
  }

  static int gc(lua_State* L)
  {
    asio::signal* o = static_cast<asio::signal*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~signal();
    }
    return 0;
  }
  
  static int async_wait(lua_State* L)
  {
    asio::signal* o = gce::lua::from_lua<asio::signal>(L, 1, "signal");
    
    if (lua_type(L, 2) != LUA_TNIL)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
      o->async_wait(*msg);
    }
    else
    {
      o->async_wait();
    }
    return 0;
  }
  
  static int add(lua_State* L)
  {
    asio::signal* o = gce::lua::from_lua<asio::signal>(L, 1, "signal");
    
    int sig = luaL_checkinteger(L, 2);
    
    errcode_t ec;
    (*o)->add(sig, ec);
    
    lua_pushinteger(L, ec.value());
    return 1;
  }
  
  static int remove(lua_State* L)
  {
    asio::signal* o = gce::lua::from_lua<asio::signal>(L, 1, "signal");
    
    int sig = luaL_checkinteger(L, 2);
    
    errcode_t ec;
    (*o)->remove(sig, ec);
    
    lua_pushinteger(L, ec.value());
    return 1;
  }
  
  static int cancel(lua_State* L)
  {
    asio::signal* o = gce::lua::from_lua<asio::signal>(L, 1, "signal");
    
    errcode_t ec;
    (*o)->cancel(ec);
    
    lua_pushinteger(L, ec.value());
    return 1;
  }
  
  static int clear(lua_State* L)
  {
    asio::signal* o = gce::lua::from_lua<asio::signal>(L, 1, "signal");
    
    errcode_t ec;
    (*o)->clear(ec);
    
    lua_pushinteger(L, ec.value());
    return 1;
  }
};
///------------------------------------------------------------------------------
/// tcp_option
///------------------------------------------------------------------------------
struct tcp_option
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static void create(lua_State* L, tcpopt_t const& opt = make_tcpopt())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, opt, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, opt);
#endif
  }
};

static void load(lua_State* L, int arg, tcpopt_t& opt)
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

static void push(lua_State* L, tcpopt_t const& opt)
{
  tcp_option::create(L, opt);
}
///------------------------------------------------------------------------------
/// ssl_option
///------------------------------------------------------------------------------
struct ssl_option
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static void create(lua_State* L, sslopt_t const& opt = make_sslopt())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, opt, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, opt);
#endif
  }
};

static void load(lua_State* L, int arg, sslopt_t& opt)
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

static void push(lua_State* L, sslopt_t const& opt)
{
  ssl_option::create(L, opt);
}
///------------------------------------------------------------------------------
/// ssl_context
///------------------------------------------------------------------------------
struct ssl_context
{
  explicit ssl_context(int method)
  : ctx_((boost::asio::ssl::context_base::method)method)
  {
  }

  boost::asio::ssl::context ctx_;
  std::vector<int> ref_list_;

  static int make(lua_State* L)
  {
    try
    {
      void* block = lua_newuserdata(L, sizeof(ssl_context));
      GCE_VERIFY(block != 0).msg("lua_newuserdata for ssl_context failed");

      int method = luaL_checkint(L, 1);
      ssl_context* wrap = new (block) ssl_context(method);
      boost::asio::ssl::context& ssl_ctx = wrap->ctx_;

      sslopt_t opt = make_sslopt();
      if (lua_type(L, 2) == LUA_TUSERDATA)
      {
        load(L, 2, opt);
      }

      if (lua_type(L, 3) == LUA_TFUNCTION)
      {
        lua_pushvalue(L, 3);
        int k = gce::lualib::make_ref(L, "libasio");
        wrap->ref_list_.push_back(k);

        ssl_ctx.set_password_callback(
          boost::bind(&ssl_context::password_callback, _arg1, _arg2, L, k)
          );
      }

      gce::lualib::setmetatab(L, "ssl_context");
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static int gc(lua_State* L)
  {
    ssl_context* wrap = static_cast<ssl_context*>(lua_touserdata(L, 1));
    if (wrap)
    {
      BOOST_FOREACH(int k, wrap->ref_list_)
      {
        gce::lualib::rmv_ref(L, "libasio", k);
      }
      wrap->~ssl_context();
    }
    return 0;
  }

private:
  static std::string password_callback(
    size_t max_length, 
    boost::asio::ssl::context_base::password_purpose purpose, 
    lua_State* L, int k
    )
  {
    gce::lualib::get_ref(L, "libasio", k);
    lua_pushinteger(L, (lua_Integer)max_length);
    lua_pushinteger(L, (lua_Integer)purpose);

    if (lua_pcall(L, 2, 1, 0) != 0)
    {
      return std::string();
    }

    size_t len = 0;
    char const* pwd = lua_tolstring(L, -1, &len);
    lua_pop(L, 1);
    return std::string(pwd, len);
  }
};
///------------------------------------------------------------------------------
} /// namespace lua
} /// namespace detail
} /// namespace asio
} /// namespace gce

#endif /// GCE_ASIO_TIMER_HPP
