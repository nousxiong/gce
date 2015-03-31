#ifndef service_id_adl_cpp2lua_h_
#define service_id_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>
#include <gce/actor/match.adl.c2l.h>

#include "service_id.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::adl::service_id& value)
    {
      lua_getfield(L, -1, "valid_");
      {load(L, value.valid_);lua_pop(L, 1);}
      lua_getfield(L, -1, "ctxid_");
      {load(L, value.ctxid_);lua_pop(L, 1);}
      lua_getfield(L, -1, "name_");
      {load(L, value.name_);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::adl::service_id const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 3);
      if(use_adata && !set_metatable(L, "ad_mt_gce_adl.service_id")){ luaL_error(L,"unknow type: gce_adl.service_id"); }
      {push(L, value.valid_);}
      lua_setfield(L, -2, "valid_");
      {push(L, value.ctxid_, use_adata);}
      lua_setfield(L, -2, "ctxid_");
      {push(L, value.name_, use_adata);}
      lua_setfield(L, -2, "name_");
    }

  }
}

#endif
