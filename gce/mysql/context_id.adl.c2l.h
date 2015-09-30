#ifndef context_id_adl_cpp2lua_h_
#define context_id_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "context_id.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::mysql::adl::context_id& value)
    {
      lua_getfield(L, -1, "ptr_");
      {load(L, value.ptr_);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::mysql::adl::context_id const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 1);
      if(use_adata && !set_metatable(L, "ad_mt_gce_mysql_adl.context_id")){ luaL_error(L,"unknow type: gce_mysql_adl.context_id"); }
      {push(L, value.ptr_);}
      lua_setfield(L, -2, "ptr_");
    }

  }
}

#endif
