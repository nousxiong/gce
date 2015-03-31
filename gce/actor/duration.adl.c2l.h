#ifndef duration_adl_cpp2lua_h_
#define duration_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "duration.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::adl::duration& value)
    {
      lua_getfield(L, -1, "val_");
      {load(L, value.val_);lua_pop(L, 1);}
      lua_getfield(L, -1, "ty_");
      {load(L, value.ty_);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::adl::duration const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 2);
      if(use_adata && !set_metatable(L, "ad_mt_gce_adl.duration")){ luaL_error(L,"unknow type: gce_adl.duration"); }
      {push(L, value.val_);}
      lua_setfield(L, -2, "val_");
      {push(L, value.ty_);}
      lua_setfield(L, -2, "ty_");
    }

  }
}

#endif
