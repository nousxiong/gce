#ifndef actor_id_adl_cpp2lua_h_
#define actor_id_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>
#include <gce/actor/match.adl.c2l.h>
#include <gce/actor/service_id.adl.c2l.h>

#include "actor_id.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::adl::actor_id& value)
    {
      lua_getfield(L, -1, "ctxid_");
      {load(L, value.ctxid_);lua_pop(L, 1);}
      lua_getfield(L, -1, "timestamp_");
      {load(L, value.timestamp_);lua_pop(L, 1);}
      lua_getfield(L, -1, "uintptr_");
      {load(L, value.uintptr_);lua_pop(L, 1);}
      lua_getfield(L, -1, "svc_id_");
      {load(L, value.svc_id_);lua_pop(L, 1);}
      lua_getfield(L, -1, "type_");
      {load(L, value.type_);lua_pop(L, 1);}
      lua_getfield(L, -1, "in_pool_");
      {load(L, value.in_pool_);lua_pop(L, 1);}
      lua_getfield(L, -1, "sid_");
      {load(L, value.sid_);lua_pop(L, 1);}
      lua_getfield(L, -1, "svc_");
      {load(L, value.svc_);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::adl::actor_id const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 8);
      if(use_adata && !set_metatable(L, "ad_mt_gce_adl.actor_id")){ luaL_error(L,"unknow type: gce_adl.actor_id"); }
      {push(L, value.ctxid_, use_adata);}
      lua_setfield(L, -2, "ctxid_");
      {push(L, value.timestamp_);}
      lua_setfield(L, -2, "timestamp_");
      {push(L, value.uintptr_);}
      lua_setfield(L, -2, "uintptr_");
      {push(L, value.svc_id_);}
      lua_setfield(L, -2, "svc_id_");
      {push(L, value.type_);}
      lua_setfield(L, -2, "type_");
      {push(L, value.in_pool_);}
      lua_setfield(L, -2, "in_pool_");
      {push(L, value.sid_);}
      lua_setfield(L, -2, "sid_");
      {push(L, value.svc_, use_adata);}
      lua_setfield(L, -2, "svc_");
    }

  }
}

#endif
