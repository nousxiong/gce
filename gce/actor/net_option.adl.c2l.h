#ifndef net_option_adl_cpp2lua_h_
#define net_option_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>
#include <gce/actor/duration.adl.c2l.h>

#include "net_option.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::adl::net_option& value)
    {
      lua_getfield(L, -1, "is_router");
      {load(L, value.is_router);lua_pop(L, 1);}
      lua_getfield(L, -1, "heartbeat_period");
      {load(L, value.heartbeat_period);lua_pop(L, 1);}
      lua_getfield(L, -1, "heartbeat_count");
      {load(L, value.heartbeat_count);lua_pop(L, 1);}
      lua_getfield(L, -1, "init_reconn_period");
      {load(L, value.init_reconn_period);lua_pop(L, 1);}
      lua_getfield(L, -1, "init_reconn_try");
      {load(L, value.init_reconn_try);lua_pop(L, 1);}
      lua_getfield(L, -1, "reconn_period");
      {load(L, value.reconn_period);lua_pop(L, 1);}
      lua_getfield(L, -1, "reconn_try");
      {load(L, value.reconn_try);lua_pop(L, 1);}
      lua_getfield(L, -1, "rebind_period");
      {load(L, value.rebind_period);lua_pop(L, 1);}
      lua_getfield(L, -1, "rebind_try");
      {load(L, value.rebind_try);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::adl::net_option const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 9);
      if(use_adata && !set_metatable(L, "ad_mt_gce_adl.net_option")){ luaL_error(L,"unknow type: gce_adl.net_option"); }
      {push(L, value.is_router);}
      lua_setfield(L, -2, "is_router");
      {push(L, value.heartbeat_period, use_adata);}
      lua_setfield(L, -2, "heartbeat_period");
      {push(L, value.heartbeat_count);}
      lua_setfield(L, -2, "heartbeat_count");
      {push(L, value.init_reconn_period, use_adata);}
      lua_setfield(L, -2, "init_reconn_period");
      {push(L, value.init_reconn_try);}
      lua_setfield(L, -2, "init_reconn_try");
      {push(L, value.reconn_period, use_adata);}
      lua_setfield(L, -2, "reconn_period");
      {push(L, value.reconn_try);}
      lua_setfield(L, -2, "reconn_try");
      {push(L, value.rebind_period, use_adata);}
      lua_setfield(L, -2, "rebind_period");
      {push(L, value.rebind_try);}
      lua_setfield(L, -2, "rebind_try");
    }

  }
}

#endif
