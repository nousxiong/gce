///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_REDIS_HPP
#define GCE_REDIS_HPP

#include <gce/redis/config.hpp>
#ifdef GCE_LUA
# include <gce/redis/detail/lua_wrap.hpp>
# include <sstream>
#endif

#ifdef GCE_LUA
namespace gce
{
namespace redis
{
static void make_libredis(lua_State* L)
{
  /// register libredis
  gce::lualib::open(L)
    .begin("libredis")
      .add_function("make_ctxid", detail::lua::context_id::make)
      .add_function("make_conn", detail::lua::conn::make)
      .add_function("make_session", detail::lua::session::make)
      .begin_userdata("session")
        .add_function("open", detail::lua::session::open)
        .add_function("cmd", detail::lua::session::cmd)
        .add_function("args", detail::lua::session::args)
        .add_function("execute", detail::lua::session::execute)
        .add_function("query", detail::lua::session::query)
        .add_function("ping", detail::lua::session::ping)
        .add_function("get_snid", detail::lua::session::get_snid)
        .add_function("__gc", detail::lua::session::gc)
      .end_userdata()
      .begin_userdata("array_ref")
        .add_function("type", detail::lua::array_ref::type)
        .add_function("integer", detail::lua::array_ref::integer)
        .add_function("string", detail::lua::array_ref::string)
        .add_function("error", detail::lua::array_ref::error)
        .add_function("bulkstr", detail::lua::array_ref::bulkstr)
        .add_function("array", detail::lua::array_ref::array)
        .add_function("get", detail::lua::array_ref::get)
        .add_function("__gc", detail::lua::array_ref::gc)
      .end_userdata()
      .add_function("make_result", detail::lua::result::make)
      .begin_userdata("result")
        .add_function("type", detail::lua::result::type)
        .add_function("integer", detail::lua::result::integer)
        .add_function("string", detail::lua::result::string)
        .add_function("error", detail::lua::result::error)
        .add_function("bulkstr", detail::lua::result::bulkstr)
        .add_function("array", detail::lua::result::array)
        .add_function("get", detail::lua::result::get)
        .add_function("gcety", detail::lua::result::gcety)
        .add_function("__gc", detail::lua::result::gc)
      .end_userdata()
      .add_function("init_nil", detail::lua::init_nil)
    .end()
    ;

  /// init libasio
  std::ostringstream oss;
  oss << "local libredis = require('libredis')" << std::endl;

  /// types
  oss << "libredis.ty_result = " << gce::redis::lua::ty_result << std::endl;

  /// resp types
  oss << "libredis.resp_null = " << resp::ty_null << std::endl;
  oss << "libredis.resp_string = " << resp::ty_string << std::endl;
  oss << "libredis.resp_error = " << resp::ty_error << std::endl;
  oss << "libredis.resp_integer = " << resp::ty_integer << std::endl;
  oss << "libredis.resp_bulkstr = " << resp::ty_bulkstr << std::endl;
  oss << "libredis.resp_array = " << resp::ty_array << std::endl;

  std::string init_libredis_script = oss.str();
  if (luaL_dostring(L, init_libredis_script.c_str()) != 0)
  {
    std::string errmsg("gce::lua_exception: ");
    errmsg += lua_tostring(L, -1);
    GCE_VERIFY(false).msg(errmsg.c_str()).except<gce::lua_exception>();
  }
}
}
}
#endif

#endif /// GCE_REDIS_HPP
