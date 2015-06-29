#ifndef sn_option_adl_cpp2lua_h_
#define sn_option_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>
#include <gce/actor/duration.adl.c2l.h>

#include "sn_option.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::asio::adl::sn_option& value)
    {
      lua_getfield(L, -1, "idle_period");
      {load(L, value.idle_period);lua_pop(L, 1);}
      lua_getfield(L, -1, "bigmsg_size");
      {load(L, value.bigmsg_size);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::asio::adl::sn_option const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 2);
      if(use_adata && !set_metatable(L, "ad_mt_gce_asio_adl.sn_option")){ luaL_error(L,"unknow type: gce_asio_adl.sn_option"); }
      {push(L, value.idle_period, use_adata);}
      lua_setfield(L, -2, "idle_period");
      {push(L, value.bigmsg_size);}
      lua_setfield(L, -2, "bigmsg_size");
    }

  }
}

#endif
