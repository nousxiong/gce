#ifndef gce_mysql_adl_conn_adl_cpp2lua_h_
#define gce_mysql_adl_conn_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "conn.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::mysql::adl::conn& value)
    {
      lua_getfield(L, -1, "ptr_");
      {load(L, value.ptr_);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::mysql::adl::conn const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 1);
      if(use_adata && !set_metatable(L, "ad.mt.gce.mysql.adl.conn")){ luaL_error(L,"unknow type: gce.mysql.adl.conn"); }
      {push(L, value.ptr_);}
      lua_setfield(L, -2, "ptr_");
    }

  }
}

#endif
