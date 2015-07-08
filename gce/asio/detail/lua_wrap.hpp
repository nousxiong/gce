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
#include <gce/asio/serial_port.hpp>
#include <gce/asio/tcp/resolver.hpp>
#include <gce/asio/tcp/acceptor.hpp>
#include <gce/asio/tcp/socket.hpp>
#ifdef GCE_OPENSSL
# include <gce/asio/ssl/stream.hpp>
#endif 
#include <gce/asio/spt_option.hpp>
#include <gce/asio/spt_option.adl.c2l.h>
#include <gce/asio/tcp/option.hpp>
#include <gce/asio/tcp/tcp_option.adl.c2l.h>
#ifdef GCE_OPENSSL
# include <gce/asio/ssl/option.hpp>
# include <gce/asio/ssl/ssl_option.adl.c2l.h>
#endif
#include <gce/asio/session.hpp>
#include <gce/asio/sn_option.hpp>
#include <gce/asio/sn_option.adl.c2l.h>
#include <gce/asio/parser/simple.hpp>
#include <gce/asio/lua_object.hpp>
#include <gce/lualib/all.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/polymorphic_cast.hpp>
#include <boost/array.hpp>
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

    impl_t* o = new (block) impl_t(*a);

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
    impl_t* o = gce::lua::from_lua<impl_t>(L, 1, type().c_str());

    gce::duration_t dur;
    gce::lua::load(L, 2, dur);

    if (lua_type(L, 3) == LUA_TUSERDATA)
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
    
    if (lua_type(L, 2) == LUA_TUSERDATA)
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
/// spt_option
///------------------------------------------------------------------------------
struct spt_option
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static void create(lua_State* L, sptopt_t const& opt = make_sptopt())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, opt, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, opt);
#endif
  }
};

static void load(lua_State* L, int arg, sptopt_t& opt)
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

static void push(lua_State* L, sptopt_t const& opt)
{
  spt_option::create(L, opt);
}
///------------------------------------------------------------------------------
/// serial_port
///------------------------------------------------------------------------------
struct serial_port
{
  static int make(lua_State* L)
  {
    try
    {
      gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
      void* block = lua_newuserdata(L, sizeof(asio::serial_port));
      if (!block)
      {
        return luaL_error(L, "lua_newuserdata for serial_port failed");
      }

      asio::serial_port* o = 0;
      if (lua_type(L, 2) == LUA_TSTRING)
      {
        char const* device = lua_tostring(L, 2);
        o = new (block) asio::serial_port(*a, device);

        int ty = lua_type(L, 3);
        if (ty != LUA_TNONE || ty != LUA_TNIL)
        {
          sptopt_t opt;
          load(L, 3, opt);

          typedef boost::asio::serial_port_base spt_base_t;
          if (opt.baud_rate != -1)
          {
            (*o)->set_option(spt_base_t::baud_rate((unsigned int)opt.baud_rate));
          }
          if (opt.flow_control != -1)
          {
            (*o)->set_option(spt_base_t::flow_control((spt_base_t::flow_control::type)opt.flow_control));
          }
          if (opt.parity != -1)
          {
            (*o)->set_option(spt_base_t::parity((spt_base_t::parity::type)opt.parity));
          }
          if (opt.stop_bits != -1)
          {
            (*o)->set_option(spt_base_t::stop_bits((spt_base_t::stop_bits::type)opt.stop_bits));
          }
          if (opt.character_size != -1)
          {
            (*o)->set_option(spt_base_t::character_size((unsigned int)opt.character_size));
          }
        }
      }
      else
      {
        o = new (block) asio::serial_port(*a);
      }

      lua_pushvalue(L, -1);
      int k = gce::lualib::make_ref(L, "libasio");
      (*a)->add_addon("libasio", k, o);
      gce::lualib::setmetatab(L, "serial_port");
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static int gc(lua_State* L)
  {
    asio::serial_port* o = static_cast<asio::serial_port*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~serial_port();
    }
    return 0;
  }

  static int async_read(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    size_t length = (size_t)luaL_checkinteger(L, 2);
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_read(length, *msg);
    }
    else
    {
      o->async_read(length);
    }
    return 0;
  }

  static int async_read_some(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    size_t length = (size_t)luaL_checkinteger(L, 2);
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_read_some(length, *msg);
    }
    else
    {
      o->async_read_some(length);
    }
    return 0;
  }

  static int async_write(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
      size_t length = (size_t)luaL_checkinteger(L, 3);
      o->async_write(*msg, length);
    }
    else
    {
      o->async_write(*msg);
    }
    return 0;
  }

  static int async_write_some(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
    size_t offset = 0;
    size_t length = size_nil;

    if (lua_type(L, 3) == LUA_TNUMBER)
    {
      offset = (size_t)luaL_checkinteger(L, 3);
    }
    if (lua_type(L, 4) == LUA_TNUMBER)
    {
      length = (size_t)luaL_checkinteger(L, 4);
    }

    o->async_write_some(*msg, offset, length);
    return 0;
  }

  static int open(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    char const* device = luaL_checkstring(L, 2);
    gce::errcode_t ec;
    (*o)->open(device, ec);
    lua_pushinteger(L, ec.value());
    return 1;
  }

  static int is_open(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    bool rt = (*o)->is_open();
    lua_pushboolean(L, rt != 0);
    return 1;
  }

  static int send_break(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    gce::errcode_t ec;
    (*o)->send_break(ec);
    lua_pushinteger(L, ec.value());
    return 1;
  }

  static int cancel(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    gce::errcode_t ignored_ec;
    (*o)->cancel(ignored_ec);
    return 0;
  }

  static int close(lua_State* L)
  {
    asio::serial_port* o = gce::lua::from_lua<asio::serial_port>(L, 1, "serial_port");
    gce::errcode_t ignored_ec;
    (*o)->close(ignored_ec);
    return 0;
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
#ifdef GCE_OPENSSL
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

/// ssl option helper
static long make_options(sslopt_t const& opt)
{
  long rt = -1;
  boost::array<long, 6> opts;
  size_t size = 0;

  if (opt.default_workarounds != -1)
  {
    opts[size] = opt.default_workarounds;
    ++size;
  }
  if (opt.single_dh_use != -1)
  {
    opts[size] = opt.single_dh_use;
    ++size;
  }
  if (opt.no_sslv2 != -1)
  {
    opts[size] = opt.no_sslv2;
    ++size;
  }
  if (opt.no_sslv3 != -1)
  {
    opts[size] = opt.no_sslv3;
    ++size;
  }
  if (opt.no_tlsv1 != -1)
  {
    opts[size] = opt.no_tlsv1;
    ++size;
  }
  if (opt.no_compression != -1)
  {
    opts[size] = opt.no_compression;
    ++size;
  }

  if (size > 0)
  {
    rt = opts[0];
    if (size > 1)
    {
      for (size_t i=1; i<size; ++i)
      {
        rt |= opts[i];
      }
    }
  }
  return rt;
}

static int make_verify(sslopt_t const& opt)
{
  int rt = -1;
  boost::array<long, 4> opts;
  size_t size = 0;

  if (opt.verify_none != -1)
  {
    opts[size] = opt.verify_none;
    ++size;
  }
  if (opt.verify_peer != -1)
  {
    opts[size] = opt.verify_peer;
    ++size;
  }
  if (opt.verify_fail_if_no_peer_cert != -1)
  {
    opts[size] = opt.verify_fail_if_no_peer_cert;
    ++size;
  }
  if (opt.verify_client_once != -1)
  {
    opts[size] = opt.verify_client_once;
    ++size;
  }

  if (size > 0)
  {
    rt = opts[0];
    if (size > 1)
    {
      for (size_t i=1; i<size; ++i)
      {
        rt |= opts[i];
      }
    }
  }
  return rt;
}
static bool verify_callback(bool preverified, boost::asio::ssl::verify_context& ctx, lua_State* L, int k)
{
  gce::lualib::get_ref(L, "libasio", k);
  lua_pushboolean(L, preverified);
  lua_pushlightuserdata(L, &ctx);

  if (lua_pcall(L, 2, 1, 0) != 0)
  {
    return false;
  }

  preverified = lua_toboolean(L, -1) != 0;
  lua_pop(L, 1);
  return preverified;
}
///------------------------------------------------------------------------------
/// ssl_context
///------------------------------------------------------------------------------
struct ssl_context
  : public gce::lua::basic_object
{
  typedef boost::shared_ptr<boost::asio::ssl::context> object_t;

  object_t obj_;
  std::vector<int> ref_list_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "ssl_context";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_ << ref_list_.size();
    BOOST_FOREACH(int k, ref_list_)
    {
      msg << k;
    }
  }

  virtual void unpack(gce::message& msg)
  {
    std::vector<int>::size_type size = 0;
    msg >> obj_ >> size;
    ref_list_.resize(size);
    BOOST_FOREACH(int& k, ref_list_)
    {
      msg >> k;
    }
  }

  virtual int gcety() const
  {
    return (int)gce::asio::lua::ty_ssl_context;
  }

  static int make(lua_State* L)
  {
    try
    {
      void* block = lua_newuserdata(L, sizeof(ssl_context));
      GCE_VERIFY(block != 0).msg("lua_newuserdata for ssl_context failed");

      ssl_context* wrap = new (block) ssl_context;
      if (lua_type(L, 1) == LUA_TNUMBER)
      {
        int method = lua_tointeger(L, 1);
        wrap->obj_ = boost::make_shared<boost::asio::ssl::context>((boost::asio::ssl::context_base::method)method);
        boost::asio::ssl::context& ssl_ctx = *wrap->obj_;

        sslopt_t opt = make_sslopt();
        if (lua_type(L, 2) == LUA_TTABLE)
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

        if (opt.default_verify_paths != -1)
        {
          ssl_ctx.set_default_verify_paths();
        }
        else
        {
          BOOST_FOREACH(std::string const& path, opt.verify_paths)
          {
            ssl_ctx.add_verify_path(path);
          }
        }

        if (opt.verify_depth != -1)
        {
          ssl_ctx.set_verify_depth((int)opt.verify_depth);
        }

        long opts = make_options(opt);
        if (opts != -1)
        {
          ssl_ctx.set_options(opts);
        }

        int vm = make_verify(opt);
        if (vm != -1)
        {
          ssl_ctx.set_verify_mode(vm);
        }

        if (!opt.certificate_authority.empty())
        {
          ssl_ctx.add_certificate_authority(
            boost::asio::buffer(
              opt.certificate_authority.data(), 
              opt.certificate_authority.size()
              )
            );
        }
        else if (!opt.verify_file.empty())
        {
          ssl_ctx.load_verify_file(opt.verify_file);
        }

        if (!opt.certificate.empty())
        {
          ssl_ctx.use_certificate(
            boost::asio::buffer(
              opt.certificate.data(), 
              opt.certificate.size()
              ),
            (boost::asio::ssl::context_base::file_format)opt.certificate_format
            );
        }
        else if (!opt.certificate_file.empty())
        {
          ssl_ctx.use_certificate_file(
            opt.certificate_file, 
            (boost::asio::ssl::context_base::file_format)opt.certificate_format
            );
        }

        if (!opt.certificate_chain.empty())
        {
          ssl_ctx.use_certificate_chain(
            boost::asio::buffer(
              opt.certificate_chain.data(), 
              opt.certificate_chain.size()
              )
            );
        }
        else if (!opt.certificate_chain_file.empty())
        {
          ssl_ctx.use_certificate_chain_file(opt.certificate_chain_file);
        }

        if (!opt.private_key.empty())
        {
          ssl_ctx.use_private_key(
            boost::asio::buffer(
              opt.private_key.data(), 
              opt.private_key.size()
              ),
            (boost::asio::ssl::context_base::file_format)opt.private_key_format
            );
        }
        else if (!opt.private_key_file.empty())
        {
          ssl_ctx.use_private_key_file(
            opt.private_key_file, 
            (boost::asio::ssl::context_base::file_format)opt.private_key_format
            );
        }

        if (!opt.rsa_private_key.empty())
        {
          ssl_ctx.use_rsa_private_key(
            boost::asio::buffer(
              opt.rsa_private_key.data(), 
              opt.rsa_private_key.size()
              ),
            (boost::asio::ssl::context_base::file_format)opt.rsa_private_key_format
            );
        }
        else if (!opt.rsa_private_key_file.empty())
        {
          ssl_ctx.use_rsa_private_key_file(
            opt.rsa_private_key_file, 
            (boost::asio::ssl::context_base::file_format)opt.rsa_private_key_format
            );
        }

        if (!opt.tmp_dh.empty())
        {
          ssl_ctx.use_tmp_dh(boost::asio::buffer(opt.tmp_dh.data(), opt.tmp_dh.size()));
        }
        else if (!opt.tmp_dh_file.empty())
        {
          ssl_ctx.use_tmp_dh_file(opt.tmp_dh_file);
        }
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

  static int gcety(lua_State* L)
  {
    lua_pushinteger(L, (int)gce::asio::lua::ty_ssl_context);
    return 1;
  }

  static int set_verify_callback(lua_State* L)
  {
    ssl_context* wrap = gce::lua::from_lua<ssl_context>(L, 1, "ssl_context");
    luaL_argcheck(L, lua_type(L, 2) == LUA_TFUNCTION, 2, "'function' expected");
    boost::asio::ssl::context& ssl_ctx = *wrap->obj_;

    lua_pushvalue(L, 2);
    int k = gce::lualib::make_ref(L, "libasio");
    wrap->ref_list_.push_back(k);

    ssl_ctx.set_verify_callback(boost::bind(&verify_callback, _arg1, _arg2, L, k));
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
#endif /// GCE_OPENSSL
///------------------------------------------------------------------------------
/// tcp_endpoint
///------------------------------------------------------------------------------
struct tcp_endpoint
  : public gce::lua::basic_object
{
  typedef boost::asio::ip::tcp::endpoint tcp_endpoint_t;
  typedef boost::asio::ip::tcp::resolver::iterator tcp_endpoint_itr;
  typedef boost::shared_ptr<tcp_endpoint_t> object_t;

  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "tcp_endpoint";
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
    return gce::asio::lua::ty_tcp_endpoint;
  }

  static int make(lua_State* L)
  {
    try
    {
      void* block = lua_newuserdata(L, sizeof(tcp_endpoint));
      GCE_VERIFY(block != 0).msg("lua_newuserdata for tcp_endpoint failed");

      tcp_endpoint* wrap = new (block) tcp_endpoint;

      if (lua_type(L, 1) == LUA_TSTRING)
      {
        char const* addr = lua_tostring(L, 1);
        uint16_t port = (uint16_t)luaL_checkinteger(L, 2);
        boost::asio::ip::address address;
        address.from_string(addr);
        wrap->obj_ = boost::make_shared<tcp_endpoint_t>(boost::ref(address), boost::ref(port));
      }
      else
      {
        wrap->obj_ = boost::make_shared<tcp_endpoint_t>();
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
    tcp_endpoint* wrap = static_cast<tcp_endpoint*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~tcp_endpoint();
    }
    return 0;
  }

  static int gcety(lua_State* L)
  {
    tcp_endpoint* wrap = gce::lua::from_lua<tcp_endpoint>(L, 1, name());
    lua_pushinteger(L, wrap->gcety());
    return 1;
  }
};

///------------------------------------------------------------------------------
/// tcp_endpoint_itr
///------------------------------------------------------------------------------
struct tcp_endpoint_itr
  : public gce::lua::basic_object
{
  typedef boost::asio::ip::tcp::resolver::iterator tcp_endpoint_itr_t;
  typedef boost::shared_ptr<tcp_endpoint_itr_t> object_t;

  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "tcp_endpoint_itr";
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
    return gce::asio::lua::ty_tcp_endpoint_itr;
  }

  static int make(lua_State* L)
  {
    void* block = lua_newuserdata(L, sizeof(tcp_endpoint_itr));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for tcp_endpoint_itr failed");
    }

    tcp_endpoint_itr* wrap = new (block) tcp_endpoint_itr;
    wrap->obj_ = boost::make_shared<tcp_endpoint_itr_t>();
    gce::lualib::setmetatab(L, name());
    return 1;
  }

  static int gc(lua_State* L)
  {
    tcp_endpoint* wrap = static_cast<tcp_endpoint*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~tcp_endpoint();
    }
    return 0;
  }

  static int gcety(lua_State* L)
  {
    tcp_endpoint* wrap = gce::lua::from_lua<tcp_endpoint>(L, 1, name());
    lua_pushinteger(L, wrap->gcety());
    return 1;
  }
};
///------------------------------------------------------------------------------
/// tcp socket impl
///------------------------------------------------------------------------------
struct tcp_socket_impl
  : public gce::lua::basic_object
{
  typedef boost::asio::ip::tcp::socket tcp_socket_t;
  typedef boost::shared_ptr<tcp_socket_t> object_t;
  object_t obj_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "tcp_socket_impl";
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
    return gce::asio::lua::ty_tcp_socket_impl;
  }

  static int make(lua_State* L)
  {
    gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");

    void* block = lua_newuserdata(L, sizeof(tcp_socket_impl));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for tcp_socket_impl failed");
    }

    gce::io_service_t& ios = (*a)->get_context().get_io_service();
    tcp_socket_impl* wrap = new (block) tcp_socket_impl;
    wrap->obj_ = boost::make_shared<tcp_socket_t>(boost::ref(ios));
    gce::lualib::setmetatab(L, name());
    return 1;
  }

  static int gc(lua_State* L)
  {
    tcp_socket_impl* wrap = static_cast<tcp_socket_impl*>(lua_touserdata(L, 1));
    if (wrap)
    {
      wrap->~tcp_socket_impl();
    }
    return 0;
  }

  static int gcety(lua_State* L)
  {
    tcp_socket_impl* wrap = gce::lua::from_lua<tcp_socket_impl>(L, 1, name());
    lua_pushinteger(L, wrap->gcety());
    return 1;
  }
};
#ifdef GCE_OPENSSL
///------------------------------------------------------------------------------
/// ssl stream impl
///------------------------------------------------------------------------------
struct ssl_stream_impl
  : public gce::lua::basic_object
{
  typedef boost::asio::ip::tcp::socket stream_t;
  typedef boost::asio::ssl::stream<stream_t> ssl_socket_t;
  typedef boost::shared_ptr<ssl_socket_t> object_t;
  object_t obj_;
  std::vector<int> ref_list_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "ssl_stream_impl";
  }

  virtual void pack(gce::message& msg)
  {
    msg << obj_ << ref_list_.size();
    BOOST_FOREACH(int k, ref_list_)
    {
      msg << k;
    }
  }

  virtual void unpack(gce::message& msg)
  {
    std::vector<int>::size_type size = 0;
    msg >> obj_ >> size;
    ref_list_.resize(size);
    BOOST_FOREACH(int& k, ref_list_)
    {
      msg >> k;
    }
  }

  virtual int gcety() const
  {
    return (int)gce::asio::lua::ty_ssl_stream_impl;
  }

  static int make(lua_State* L)
  {
    try
    {
      gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");

      void* block = lua_newuserdata(L, sizeof(ssl_stream_impl));
      GCE_VERIFY(block != 0).msg("lua_newuserdata for ssl_stream_impl failed");

      ssl_stream_impl* wrap = 0;
      int ty2 = lua_type(L, 2);
      if (ty2 == LUA_TUSERDATA)
      {
        ssl_context::object_t* ssl_ctx = gce::lua::from_lua<ssl_context>(L, 2);
        gce::io_service_t& ios = (*a)->get_context().get_io_service();
        wrap = new (block) ssl_stream_impl;
        wrap->obj_ = boost::make_shared<ssl_socket_t>(boost::ref(ios), boost::ref(**ssl_ctx));
      }
      else
      {
        wrap = new (block) ssl_stream_impl;
      }

      if (lua_type(L, 3) == LUA_TTABLE)
      {
        GCE_VERIFY(ty2 == LUA_TUSERDATA);

        object_t* o = wrap->object();
        sslopt_t opt = make_sslopt();
        load(L, 3, opt);

        if (opt.verify_depth != -1)
        {
          (*o)->set_verify_depth((int)opt.verify_depth);
        }

        int vm = make_verify(opt);
        if (vm != -1)
        {
          (*o)->set_verify_mode(vm);
        }
      }

      gce::lualib::setmetatab(L, name());
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static int gc(lua_State* L)
  {
    ssl_stream_impl* wrap = static_cast<ssl_stream_impl*>(lua_touserdata(L, 1));
    if (wrap)
    {
      BOOST_FOREACH(int k, wrap->ref_list_)
      {
        gce::lualib::rmv_ref(L, "libasio", k);
      }
      wrap->~ssl_stream_impl();
    }
    return 0;
  }

  static int gcety(lua_State* L)
  {
    ssl_stream_impl* wrap = gce::lua::from_lua<ssl_stream_impl>(L, 1, name());
    lua_pushinteger(L, wrap->gcety());
    return 1;
  }

  static int set_verify_callback(lua_State* L)
  {
    ssl_stream_impl* wrap = gce::lua::from_lua<ssl_stream_impl>(L, 1, name());
    luaL_argcheck(L, lua_type(L, 2) == LUA_TFUNCTION, 2, "'function' expected");
    object_t* o = wrap->object();

    lua_pushvalue(L, 2);
    int k = gce::lualib::make_ref(L, "libasio");
    wrap->ref_list_.push_back(k);

    (*o)->set_verify_callback(boost::bind(&verify_callback, _arg1, _arg2, L, k));
    return 0;
  }
};
#endif /// GCE_OPENSSL
///------------------------------------------------------------------------------
/// tcp_resolver
///------------------------------------------------------------------------------
struct tcp_resolver
{
  typedef boost::asio::ip::tcp::resolver tcp_resolver_t;
  static int make(lua_State* L)
  {
    gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
    void* block = lua_newuserdata(L, sizeof(asio::tcp::resolver));
    GCE_VERIFY(block != 0).msg("lua_newuserdata for tcp_resolver failed");

    asio::tcp::resolver* o = new (block) asio::tcp::resolver(*a);

    lua_pushvalue(L, -1);
    int k = gce::lualib::make_ref(L, "libasio");
    (*a)->add_addon("libasio", k, o);
    gce::lualib::setmetatab(L, "tcp_resolver");
    return 1;
  }

  static int gc(lua_State* L)
  {
    asio::tcp::resolver* o = static_cast<asio::tcp::resolver*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~resolver();
    }
    return 0;
  }

  static int async_resolve(lua_State* L)
  {
    asio::tcp::resolver* o = gce::lua::from_lua<asio::tcp::resolver>(L, 1, "tcp_resolver");
    size_t host_len = 0;
    char const* host = luaL_checklstring(L, 2, &host_len);
    size_t service_len = 0;
    char const* service = luaL_checklstring(L, 3, &service_len);

    asio::tcp::resolver::impl_t::query qry(
      std::string(host, host_len), 
      std::string(service, service_len)
      );
    if (lua_type(L, 4) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 4);
      o->async_resolve(qry, *msg);
    }
    else
    {
      o->async_resolve(qry);
    }
    return 0;
  }

  static int cancel(lua_State* L)
  {
    asio::tcp::resolver* o = gce::lua::from_lua<asio::tcp::resolver>(L, 1, "tcp_resolver");
    (*o)->cancel();
    return 0;
  }
};
///------------------------------------------------------------------------------
/// tcp_acceptor
///------------------------------------------------------------------------------
struct tcp_acceptor
{
  typedef boost::asio::ip::tcp::socket tcp_socket_t;
  static int make(lua_State* L)
  {
    gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
    void* block = lua_newuserdata(L, sizeof(asio::tcp::acceptor));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for tcp_acceptor failed");
    }

    asio::tcp::acceptor* o = new (block) asio::tcp::acceptor(*a);

    lua_pushvalue(L, -1);
    int k = gce::lualib::make_ref(L, "libasio");
    (*a)->add_addon("libasio", k, o);
    gce::lualib::setmetatab(L, "tcp_acceptor");
    return 1;
  }

  static int gc(lua_State* L)
  {
    asio::tcp::acceptor* o = static_cast<asio::tcp::acceptor*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~acceptor();
    }
    return 0;
  }

  static int bind(lua_State* L)
  {
    try
    {
      asio::tcp::acceptor* o = gce::lua::from_lua<asio::tcp::acceptor>(L, 1, "tcp_acceptor");
      gce::lua::basic_object* bo = gce::lua::from_lua<gce::lua::basic_object>(L, 2, "endpoint");
      tcpopt_t opt = make_tcpopt();
      if (lua_type(L, 3) == LUA_TTABLE)
      {
        load(L, 3, opt);
      }

      boost::asio::ip::tcp::endpoint ep;
      if (bo->gcety() == gce::asio::lua::ty_tcp_endpoint)
      {
        tcp_endpoint* tcp_ep = boost::polymorphic_downcast<tcp_endpoint*>(bo);
        ep = *tcp_ep->object()->get();
      }
      else
      {
        tcp_endpoint_itr* itr = boost::polymorphic_downcast<tcp_endpoint_itr*>(bo);
        ep = *(*itr->object()->get());
      }

      (*o)->open(ep.protocol());

      if (opt.reuse_address != -1)
      {
        (*o)->set_option(boost::asio::socket_base::reuse_address(opt.reuse_address != 0));
      }
      (*o)->bind(ep);

      if (opt.receive_buffer_size != -1)
      {
        (*o)->set_option(boost::asio::socket_base::receive_buffer_size(opt.receive_buffer_size));
      }

      if (opt.send_buffer_size != -1)
      {
        (*o)->set_option(boost::asio::socket_base::send_buffer_size(opt.send_buffer_size));
      }

      if (opt.backlog != -1)
      {
        (*o)->listen(opt.backlog);
      }
      else
      {
        (*o)->listen(boost::asio::socket_base::max_connections);
      }

      if (opt.no_delay != -1)
      {
        (*o)->set_option(boost::asio::ip::tcp::no_delay(opt.no_delay != 0));
      }
      if (opt.keep_alive != -1)
      {
        (*o)->set_option(boost::asio::socket_base::keep_alive(opt.keep_alive != 0));
      }
      if (opt.enable_connection_aborted != -1)
      {
        (*o)->set_option(boost::asio::socket_base::enable_connection_aborted(opt.enable_connection_aborted != 0));
      }
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 0;
  }

  static int async_accept(lua_State* L)
  {
    asio::tcp::acceptor* o = gce::lua::from_lua<asio::tcp::acceptor>(L, 1, "tcp_acceptor");
    gce::lua::basic_object* bo = gce::lua::from_lua<gce::lua::basic_object>(L, 2, "socket");

    gce::message m(tcp::as_accept);
    gce::message* msg = &m;
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      msg = gce::lua::from_lua<gce::lua::message>(L, 3);
    }

    int ty = bo->gcety();
    switch (ty)
    {
    case gce::asio::lua::ty_tcp_socket_impl:
    {
      tcp_socket_impl* skt = boost::polymorphic_downcast<tcp_socket_impl*>(bo);
      o->async_accept(*skt->object()->get(), *msg);
    }break;
#ifdef GCE_OPENSSL
    case gce::asio::lua::ty_ssl_stream_impl:
    {
      ssl_stream_impl* skt = boost::polymorphic_downcast<ssl_stream_impl*>(bo);
      o->async_accept((*skt->object())->lowest_layer(), *msg);
    }break;
#endif
    default: return luaL_error(L, "accept socket type error"); break;
    }
    return 0;
  }

  static int close(lua_State* L)
  {
    asio::tcp::acceptor* o = gce::lua::from_lua<asio::tcp::acceptor>(L, 1, "tcp_acceptor");
    gce::errcode_t ignored_ec;
    (*o)->close(ignored_ec);
    return 0;
  }
};
///------------------------------------------------------------------------------
/// tcp_socket
///------------------------------------------------------------------------------
struct tcp_socket
{
  static int make(lua_State* L)
  {
    gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
    void* block = lua_newuserdata(L, sizeof(asio::tcp::socket));
    if (!block)
    {
      return luaL_error(L, "lua_newuserdata for tcp_socket failed");
    }

    asio::tcp::socket* o = 0;
    if (lua_type(L, 2) == LUA_TUSERDATA)
    {
      tcp_socket_impl::object_t* impl = gce::lua::from_lua<tcp_socket_impl>(L, 2);
      o = new (block) asio::tcp::socket(*a, *impl);
    }
    else
    {
      o = new (block) asio::tcp::socket(*a);
    }

    lua_pushvalue(L, -1);
    int k = gce::lualib::make_ref(L, "libasio");
    (*a)->add_addon("libasio", k, o);
    gce::lualib::setmetatab(L, "tcp_socket");
    return 1;
  }

  static int gc(lua_State* L)
  {
    asio::tcp::socket* o = static_cast<asio::tcp::socket*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~socket();
    }
    return 0;
  }

  static int async_connect(lua_State* L)
  {
    asio::tcp::socket* o = gce::lua::from_lua<asio::tcp::socket>(L, 1, "tcp_socket");
    gce::lua::basic_object* bo = gce::lua::from_lua<gce::lua::basic_object>(L, 2, "endpoint");
    gce::message m(tcp::as_conn);
    gce::message* msg = &m;

    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      msg = gce::lua::from_lua<gce::lua::message>(L, 3);
    }

    if (bo->gcety() == gce::asio::lua::ty_tcp_endpoint)
    {
      tcp_endpoint* tcp_ep = boost::polymorphic_downcast<tcp_endpoint*>(bo);
      o->async_connect(*tcp_ep->object()->get(), *msg);
    }
    else
    {
      tcp_endpoint_itr* itr = boost::polymorphic_downcast<tcp_endpoint_itr*>(bo);
      o->async_connect(*itr->object()->get(), *msg);
    }
    return 0;
  }

  static int async_read(lua_State* L)
  {
    asio::tcp::socket* o = gce::lua::from_lua<asio::tcp::socket>(L, 1, "tcp_socket");
    size_t length = (size_t)luaL_checkinteger(L, 2);
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_read(length, *msg);
    }
    else
    {
      o->async_read(length);
    }
    return 0;
  }

  static int async_read_some(lua_State* L)
  {
    asio::tcp::socket* o = gce::lua::from_lua<asio::tcp::socket>(L, 1, "tcp_socket");
    size_t length = (size_t)luaL_checkinteger(L, 2);
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_read_some(length, *msg);
    }
    else
    {
      o->async_read_some(length);
    }
    return 0;
  }

  static int async_write(lua_State* L)
  {
    asio::tcp::socket* o = gce::lua::from_lua<asio::tcp::socket>(L, 1, "tcp_socket");
    gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
      size_t length = (size_t)luaL_checkinteger(L, 3);
      o->async_write(*msg, length);
    }
    else
    {
      o->async_write(*msg);
    }
    return 0;
  }

  static int async_write_some(lua_State* L)
  {
    asio::tcp::socket* o = gce::lua::from_lua<asio::tcp::socket>(L, 1, "tcp_socket");
    gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
    size_t offset = 0;
    size_t length = size_nil;

    if (lua_type(L, 3) == LUA_TNUMBER)
    {
      offset = (size_t)luaL_checkinteger(L, 3);
    }
    if (lua_type(L, 4) == LUA_TNUMBER)
    {
      length = (size_t)luaL_checkinteger(L, 4);
    }

    o->async_write_some(*msg, offset, length);
    return 0;
  }

  static int close(lua_State* L)
  {
    asio::tcp::socket* o = gce::lua::from_lua<asio::tcp::socket>(L, 1, "tcp_socket");
    gce::errcode_t ignored_ec;
    (*o)->close(ignored_ec);
    return 0;
  }
};
#ifdef GCE_OPENSSL
///------------------------------------------------------------------------------
/// ssl_stream
///------------------------------------------------------------------------------
struct ssl_stream
{
  typedef boost::asio::ip::tcp::socket tcp_socket_t;
  typedef boost::asio::ssl::stream<tcp_socket_t> ssl_socket_t;
  typedef asio::ssl::stream<tcp_socket_t> object_t;

  object_t obj_;
  std::vector<int> ref_list_;

  object_t* object()
  {
    return &obj_;
  }

  static char const* name()
  {
    return "ssl_stream";
  }

  template <typename Actor, typename Context>
  ssl_stream(Actor& a, Context& ctx)
    : obj_(a, ctx)
  {
  }

  template <typename Actor>
  ssl_stream(Actor& a, boost::shared_ptr<ssl_socket_t> impl)
    : obj_(a, impl)
  {
  }

  static int make(lua_State* L)
  {
    try
    {
      gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
      void* block = lua_newuserdata(L, sizeof(ssl_stream));
      GCE_VERIFY(block != 0).msg("lua_newuserdata for tcp_socket failed");

      ssl_stream* wrap = 0;
      bool is_ssl_ctx = lua_toboolean(L, 2) != 0;
      if (is_ssl_ctx)
      {
        ssl_context::object_t* ssl_ctx = gce::lua::from_lua<ssl_context>(L, 3);
        wrap = new (block) ssl_stream(*a, **ssl_ctx);
      }
      else
      {
        ssl_stream_impl::object_t* impl = gce::lua::from_lua<ssl_stream_impl>(L, 3);
        wrap = new (block) ssl_stream(*a, *impl);
      }

      object_t* o = wrap->object();

      sslopt_t opt = make_sslopt();
      if (lua_type(L, 4) == LUA_TTABLE)
      {
        load(L, 4, opt);
      }

      if (opt.verify_depth != -1)
      {
        (*o)->set_verify_depth((int)opt.verify_depth);
      }

      int vm = make_verify(opt);
      if (vm != -1)
      {
        (*o)->set_verify_mode(vm);
      }

      lua_pushvalue(L, -1);
      int k = gce::lualib::make_ref(L, "libasio");
      (*a)->add_addon("libasio", k, o);
      gce::lualib::setmetatab(L, name());
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static int gc(lua_State* L)
  {
    ssl_stream* wrap = static_cast<ssl_stream*>(lua_touserdata(L, 1));
    if (wrap)
    {
      BOOST_FOREACH(int k, wrap->ref_list_)
      {
        gce::lualib::rmv_ref(L, "libasio", k);
      }
      wrap->~ssl_stream();
    }
    return 0;
  }

  static int set_verify_callback(lua_State* L)
  {
    ssl_stream* wrap = gce::lua::from_lua<ssl_stream>(L, 1, name());
    luaL_argcheck(L, lua_type(L, 2) == LUA_TFUNCTION, 2, "'function' expected");
    object_t* o = wrap->object();

    lua_pushvalue(L, 2);
    int k = gce::lualib::make_ref(L, "libasio");
    wrap->ref_list_.push_back(k);

    (*o)->set_verify_callback(boost::bind(&verify_callback, _arg1, _arg2, L, k));
    return 0;
  }

  static int async_connect(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    gce::lua::basic_object* bo = gce::lua::from_lua<gce::lua::basic_object>(L, 2, "endpoint");
    gce::message m(tcp::as_conn);
    gce::message* msg = &m;

    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      msg = gce::lua::from_lua<gce::lua::message>(L, 3);
    }

    if (bo->gcety() == gce::asio::lua::ty_tcp_endpoint)
    {
      tcp_endpoint* tcp_ep = boost::polymorphic_downcast<tcp_endpoint*>(bo);
      o->async_connect(*tcp_ep->object()->get(), *msg);
    }
    else
    {
      tcp_endpoint_itr* itr = boost::polymorphic_downcast<tcp_endpoint_itr*>(bo);
      o->async_connect(*itr->object()->get(), *msg);
    }
    return 0;
  }

  static int async_handshake(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    int shakety = (int)luaL_checkinteger(L, 2);
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_handshake((boost::asio::ssl::stream_base::handshake_type)shakety, *msg);
    }
    else
    {
      o->async_handshake((boost::asio::ssl::stream_base::handshake_type)shakety);
    }
    return 0;
  }

  static int async_shutdown(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    if (lua_type(L, 2) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
      o->async_shutdown(*msg);
    }
    else
    {
      o->async_shutdown();
    }
    return 0;
  }

  static int async_read(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    size_t length = (size_t)luaL_checkinteger(L, 2);
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_read(length, *msg);
    }
    else
    {
      o->async_read(length);
    }
    return 0;
  }

  static int async_read_some(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    size_t length = (size_t)luaL_checkinteger(L, 2);
    if (lua_type(L, 3) == LUA_TUSERDATA)
    {
      gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 3);
      o->async_read_some(length, *msg);
    }
    else
    {
      o->async_read_some(length);
    }
    return 0;
  }

  static int async_write(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
    if (lua_type(L, 3) == LUA_TNUMBER)
    {
      size_t length = (size_t)luaL_checkinteger(L, 3);
      o->async_write(*msg, length);
    }
    else
    {
      o->async_write(*msg);
    }
    return 0;
  }

  static int async_write_some(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
    size_t offset = 0;
    size_t length = size_nil;

    if (lua_type(L, 3) == LUA_TNUMBER)
    {
      offset = (size_t)luaL_checkinteger(L, 3);
    }
    if (lua_type(L, 4) == LUA_TNUMBER)
    {
      length = (size_t)luaL_checkinteger(L, 4);
    }

    o->async_write_some(*msg, offset, length);
    return 0;
  }

  static int close_lowest_layer(lua_State* L)
  {
    ssl_stream::object_t* o = gce::lua::from_lua<ssl_stream>(L, 1);
    gce::errcode_t ignored_ec;
    (*o)->lowest_layer().close(ignored_ec);
    return 0;
  }
};
#endif /// GCE_OPENSSL
///------------------------------------------------------------------------------
/// sn_option
///------------------------------------------------------------------------------
struct sn_option
{
  static int make(lua_State* L)
  {
    create(L);
    return 1;
  }

  static void create(lua_State* L, snopt_t const& opt = make_snopt())
  {
#if GCE_PACKER == GCE_AMSG
    adata::lua::push(L, opt, false);
#elif GCE_PACKER == GCE_ADATA
    adata::lua::push(L, opt);
#endif
  }
};

static void load(lua_State* L, int arg, snopt_t& opt)
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

static void push(lua_State* L, snopt_t const& opt)
{
  sn_option::create(L, opt);
}
///------------------------------------------------------------------------------
/// parser::simple
///------------------------------------------------------------------------------
namespace parser
{
struct simple_length
  : public asio::lua::parser::length
{
  boost::shared_ptr<asio::parser::simple> p_;

  simple_length()
    : p_(boost::make_shared<asio::parser::simple>())
  {
  }

  static int make(lua_State* L)
  {
    void* block = lua_newuserdata(L, sizeof(simple_length));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for simple_length failed");
      return 0;
    }

    new (block) simple_length;

    gce::lualib::setmetatab(L, "simple_length");
    return 1;
  }

  static int gc(lua_State* L)
  {
    simple_length* o = static_cast<simple_length*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~simple_length();
    }
    return 0;
  }

  virtual boost::shared_ptr<asio::parser::length> get_impl()
  {
    return p_;
  }
};

struct simple_regex
  : public asio::lua::parser::regex
{
  boost::shared_ptr<asio::parser::simple> p_;

  simple_regex(std::string const& r)
    : p_(boost::make_shared<asio::parser::simple>(r))
  {
  }

  static int make(lua_State* L)
  {
    void* block = lua_newuserdata(L, sizeof(simple_regex));
    if (!block)
    {
      luaL_error(L, "lua_newuserdata for simple_regex failed");
      return 0;
    }

    char const* r = luaL_checkstring(L, 1);
    new (block) simple_regex(r);

    gce::lualib::setmetatab(L, "simple_regex");
    return 1;
  }

  static int gc(lua_State* L)
  {
    simple_regex* o = static_cast<simple_regex*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~simple_regex();
    }
    return 0;
  }

  virtual boost::shared_ptr<asio::parser::regex> get_impl()
  {
    return p_;
  }
};
} /// namespace parser
///------------------------------------------------------------------------------
/// session
///------------------------------------------------------------------------------
struct session
{
  typedef boost::asio::ip::tcp::socket tcp_socket_t;
#ifdef GCE_OPENSSL
  typedef boost::asio::ssl::stream<tcp_socket_t> ssl_socket_t;
#endif
  typedef boost::asio::ip::tcp::resolver resolver_t;

  static char const* name()
  {
    return "session";
  }

  static int make(lua_State* L)
  {
    try
    {
      gce::lua::actor_proxy* a = gce::lua::from_lua<gce::lua::actor_proxy>(L, 1, "actor");
      asio::lua::parser::base* basic_parser = 
        gce::lua::from_lua<asio::lua::parser::base>(L, 2, "parser::base");
      gce::lua::basic_object* bo = 
        gce::lua::from_lua<gce::lua::basic_object>(L, 3, "socket");

      resolver_t::iterator eitr = resolver_t::iterator();
      int ty4 = lua_type(L, 4);
      if (ty4 != LUA_TNIL && ty4 != LUA_TNONE)
      {
        tcp_endpoint_itr* itr = 
          gce::lua::from_lua<tcp_endpoint_itr>(L, 4, "tcp_endpoint_itr");
        eitr = *itr->object()->get();
      }

      snopt_t opt = make_snopt();
      int ty5 = lua_type(L, 5);
      if (ty5 != LUA_TNIL && ty5 != LUA_TNONE)
      {
        load(L, 5, opt);
      }

      detail::basic_session* o = 0;
      asio::lua::parser_type pt = basic_parser->get_type();
      if (pt == asio::lua::plength)
      {
        asio::lua::parser::length* wrap = 
          static_cast<asio::lua::parser::length*>(basic_parser);
        boost::shared_ptr<asio::parser::length> p = wrap->get_impl();

        if (bo->gcety() == asio::lua::ty_tcp_socket_impl)
        {
          typedef asio::session<asio::parser::length, tcp_socket_t, gce::lua::actor_proxy> sn_t;
          o = create<tcp_socket_impl, sn_t>(L, a, p, bo, eitr, opt);
        }
        else
        {
#ifdef GCE_OPENSSL
          typedef asio::session<asio::parser::length, ssl_socket_t, gce::lua::actor_proxy> sn_t;
          o = create<ssl_stream_impl, sn_t>(L, a, p, bo, eitr, opt);
#else
          GCE_VERIFY(false).msg("no ssl support").except();
#endif
        }
      }
      else
      {
        asio::lua::parser::regex* wrap = 
          static_cast<asio::lua::parser::regex*>(basic_parser);
        boost::shared_ptr<asio::parser::regex> p = wrap->get_impl();

        if (bo->gcety() == asio::lua::ty_tcp_socket_impl)
        {
          typedef asio::session<asio::parser::regex, tcp_socket_t, gce::lua::actor_proxy> sn_t;
          o = create<tcp_socket_impl, sn_t>(L, a, p, bo, eitr, opt);
        }
        else
        {
#ifdef GCE_OPENSSL
          typedef asio::session<asio::parser::regex, ssl_socket_t, gce::lua::actor_proxy> sn_t;
          o = create<ssl_stream_impl, sn_t>(L, a, p, bo, eitr, opt);
#else
          GCE_VERIFY(false).msg("no ssl support").except();
#endif
        }
      }

      lua_pushvalue(L, -1);
      int k = gce::lualib::make_ref(L, "libasio");
      (*a)->add_addon("libasio", k, o);
      gce::lualib::setmetatab(L, name());
    }
    catch (std::exception& ex)
    {
      return luaL_error(L, ex.what());
    }
    return 1;
  }

  static int gc(lua_State* L)
  {
    detail::basic_session* o = static_cast<detail::basic_session*>(lua_touserdata(L, 1));
    if (o)
    {
      o->~basic_session();
    }
    return 0;
  }

  static int open(lua_State* L)
  {
    detail::basic_session* o = gce::lua::from_lua<detail::basic_session>(L, 1, name());
    o->open();
    return 0;
  }

  static int send(lua_State* L)
  {
    detail::basic_session* o = gce::lua::from_lua<detail::basic_session>(L, 1, name());
    gce::message* msg = gce::lua::from_lua<gce::lua::message>(L, 2);
    o->send(*msg);
    return 0;
  }

  static int close(lua_State* L)
  {
    detail::basic_session* o = gce::lua::from_lua<detail::basic_session>(L, 1, name());
    bool grateful = true;
    int ty2 = lua_type(L, 2);
    if (ty2 != LUA_TNIL || ty2 != LUA_TNONE)
    {
      grateful = lua_toboolean(L, 2) != 0;
    }
    o->close(grateful);
    return 0;
  }

  template <typename Socket, typename Session, typename Parser>
  static detail::basic_session* create(
    lua_State* L,
    gce::lua::actor_proxy* a,
    boost::shared_ptr<Parser> p, 
    gce::lua::basic_object* skt_bo, 
    resolver_t::iterator eitr,
    snopt_t opt
    )
  {
    Socket* skt = boost::polymorphic_downcast<Socket*>(skt_bo);
    void* block = lua_newuserdata(L, sizeof(Session));
    GCE_VERIFY(block != 0).msg("lua_newuserdata for session failed").except();
    return new (block) Session(*a, p, *skt->object(), eitr, opt);
  }
};
///------------------------------------------------------------------------------
} /// namespace lua
} /// namespace detail
} /// namespace asio
} /// namespace gce

#endif /// GCE_ASIO_TIMER_HPP
