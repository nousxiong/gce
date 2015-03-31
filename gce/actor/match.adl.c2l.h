#ifndef match_adl_cpp2lua_h_
#define match_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "match.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::adl::match& value)
    {
      lua_getfield(L, -1, "val_");
      {load(L, value.val_);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::adl::match const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 1);
      if(use_adata && !set_metatable(L, "ad_mt_gce_adl.match")){ luaL_error(L,"unknow type: gce_adl.match"); }
      {push(L, value.val_);}
      lua_setfield(L, -2, "val_");
    }

  }
}

#endif
