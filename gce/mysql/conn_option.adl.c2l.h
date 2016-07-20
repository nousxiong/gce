#ifndef gce_mysql_adl_conn_option_adl_cpp2lua_h_
#define gce_mysql_adl_conn_option_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "conn_option.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::mysql::adl::conn_option& value)
    {
      lua_getfield(L, -1, "init_command");
      {load(L, value.init_command);lua_pop(L, 1);}
      lua_getfield(L, -1, "compress");
      {load(L, value.compress);lua_pop(L, 1);}
      lua_getfield(L, -1, "connect_timeout");
      {load(L, value.connect_timeout);lua_pop(L, 1);}
      lua_getfield(L, -1, "read_timeout");
      {load(L, value.read_timeout);lua_pop(L, 1);}
      lua_getfield(L, -1, "reconnect");
      {load(L, value.reconnect);lua_pop(L, 1);}
      lua_getfield(L, -1, "write_timeout");
      {load(L, value.write_timeout);lua_pop(L, 1);}
      lua_getfield(L, -1, "read_default_file");
      {load(L, value.read_default_file);lua_pop(L, 1);}
      lua_getfield(L, -1, "read_default_group");
      {load(L, value.read_default_group);lua_pop(L, 1);}
      lua_getfield(L, -1, "set_charset_name");
      {load(L, value.set_charset_name);lua_pop(L, 1);}
      lua_getfield(L, -1, "client_compress");
      {load(L, value.client_compress);lua_pop(L, 1);}
      lua_getfield(L, -1, "client_found_rows");
      {load(L, value.client_found_rows);lua_pop(L, 1);}
      lua_getfield(L, -1, "client_ignore_sigpipe");
      {load(L, value.client_ignore_sigpipe);lua_pop(L, 1);}
      lua_getfield(L, -1, "client_ignore_space");
      {load(L, value.client_ignore_space);lua_pop(L, 1);}
      lua_getfield(L, -1, "client_multi_results");
      {load(L, value.client_multi_results);lua_pop(L, 1);}
      lua_getfield(L, -1, "client_multi_statements");
      {load(L, value.client_multi_statements);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::mysql::adl::conn_option const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 15);
      if(use_adata && !set_metatable(L, "ad.mt.gce.mysql.adl.conn_option")){ luaL_error(L,"unknow type: gce.mysql.adl.conn_option"); }
      {push(L, value.init_command);}
      lua_setfield(L, -2, "init_command");
      {push(L, value.compress);}
      lua_setfield(L, -2, "compress");
      {push(L, value.connect_timeout);}
      lua_setfield(L, -2, "connect_timeout");
      {push(L, value.read_timeout);}
      lua_setfield(L, -2, "read_timeout");
      {push(L, value.reconnect);}
      lua_setfield(L, -2, "reconnect");
      {push(L, value.write_timeout);}
      lua_setfield(L, -2, "write_timeout");
      {push(L, value.read_default_file);}
      lua_setfield(L, -2, "read_default_file");
      {push(L, value.read_default_group);}
      lua_setfield(L, -2, "read_default_group");
      {push(L, value.set_charset_name);}
      lua_setfield(L, -2, "set_charset_name");
      {push(L, value.client_compress);}
      lua_setfield(L, -2, "client_compress");
      {push(L, value.client_found_rows);}
      lua_setfield(L, -2, "client_found_rows");
      {push(L, value.client_ignore_sigpipe);}
      lua_setfield(L, -2, "client_ignore_sigpipe");
      {push(L, value.client_ignore_space);}
      lua_setfield(L, -2, "client_ignore_space");
      {push(L, value.client_multi_results);}
      lua_setfield(L, -2, "client_multi_results");
      {push(L, value.client_multi_statements);}
      lua_setfield(L, -2, "client_multi_statements");
    }

  }
}

#endif
