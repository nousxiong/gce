#ifndef spt_option_adl_cpp2lua_h_
#define spt_option_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "spt_option.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::asio::adl::spt_option& value)
    {
      lua_getfield(L, -1, "baud_rate");
      {load(L, value.baud_rate);lua_pop(L, 1);}
      lua_getfield(L, -1, "flow_control");
      {load(L, value.flow_control);lua_pop(L, 1);}
      lua_getfield(L, -1, "parity");
      {load(L, value.parity);lua_pop(L, 1);}
      lua_getfield(L, -1, "stop_bits");
      {load(L, value.stop_bits);lua_pop(L, 1);}
      lua_getfield(L, -1, "character_size");
      {load(L, value.character_size);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::asio::adl::spt_option const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 5);
      if(use_adata && !set_metatable(L, "ad_mt_gce_asio_adl.spt_option")){ luaL_error(L,"unknow type: gce_asio_adl.spt_option"); }
      {push(L, value.baud_rate);}
      lua_setfield(L, -2, "baud_rate");
      {push(L, value.flow_control);}
      lua_setfield(L, -2, "flow_control");
      {push(L, value.parity);}
      lua_setfield(L, -2, "parity");
      {push(L, value.stop_bits);}
      lua_setfield(L, -2, "stop_bits");
      {push(L, value.character_size);}
      lua_setfield(L, -2, "character_size");
    }

  }
}

#endif
