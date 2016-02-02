///
/// Copyright (c) 2009-2015 Nous Xiong (348944179 at qq dot com)
///
/// Distributed under the Boost Software License, Version 1.0. (See accompanying
/// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///
/// See https://github.com/nousxiong/gce for latest version.
///

#ifndef GCE_MYSQL_ASIO_HPP
#define GCE_MYSQL_ASIO_HPP

#include <gce/mysql/config.hpp>
#ifdef GCE_LUA
# include <gce/mysql/detail/lua_wrap.hpp>
# include <sstream>
#endif

#ifdef GCE_LUA
namespace gce
{
namespace mysql
{
static void make_libmysql(lua_State* L)
{
  /// register libasio
  gce::lualib::open(L)
    .begin("libmysql")
      .add_function("make_ctxid", detail::lua::context_id::make)
      .add_function("make_connopt", detail::lua::conn_option::make)
      .add_function("make_conn", detail::lua::conn::make)
      .add_function("make_session", detail::lua::session::make)
      .begin_userdata("session")
        .add_function("open", detail::lua::session::open)
        .add_function("sql", detail::lua::session::sql)
        .add_function("execute", detail::lua::session::execute)
        .add_function("ping", detail::lua::session::ping)
        .add_function("__gc", detail::lua::session::gc)
      .end_userdata()
      .add_function("make_result", detail::lua::result::make)
      .begin_userdata("result")
        .add_function("table_size", detail::lua::result::table_size)
        .add_function("row_size", detail::lua::result::row_size)
        .add_function("field_size", detail::lua::result::field_size)
        .add_function("fetch", detail::lua::result::fetch)
        .add_function("gcety", detail::lua::result::gcety)
        .add_function("__gc", detail::lua::result::gc)
      .end_userdata()
      .add_function("init_nil", detail::lua::init_nil)
    .end()
    ;

  /// init libasio
  std::ostringstream oss;
  oss << "local libmysql = require('libmysql')" << std::endl;

  /// types
  oss << "libmysql.ty_result = " << gce::mysql::lua::ty_result << std::endl;

  std::string init_libmysql_script = oss.str();
  if (luaL_dostring(L, init_libmysql_script.c_str()) != 0)
  {
    std::string errmsg("gce::lua_exception: ");
    errmsg += lua_tostring(L, -1);
    GCE_VERIFY(false).msg(errmsg.c_str()).except<gce::lua_exception>();
  }
}
}
}
#endif

#endif /// GCE_MYSQL_ASIO_HPP
