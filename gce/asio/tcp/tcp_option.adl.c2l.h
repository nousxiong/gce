#ifndef tcp_option_adl_cpp2lua_h_
#define tcp_option_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "tcp_option.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::asio::adl::tcp_option& value)
    {
      lua_getfield(L, -1, "backlog");
      {load(L, value.backlog);lua_pop(L, 1);}
      lua_getfield(L, -1, "reuse_address");
      {load(L, value.reuse_address);lua_pop(L, 1);}
      lua_getfield(L, -1, "receive_buffer_size");
      {load(L, value.receive_buffer_size);lua_pop(L, 1);}
      lua_getfield(L, -1, "send_buffer_size");
      {load(L, value.send_buffer_size);lua_pop(L, 1);}
      lua_getfield(L, -1, "no_delay");
      {load(L, value.no_delay);lua_pop(L, 1);}
      lua_getfield(L, -1, "keep_alive");
      {load(L, value.keep_alive);lua_pop(L, 1);}
      lua_getfield(L, -1, "enable_connection_aborted");
      {load(L, value.enable_connection_aborted);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::asio::adl::tcp_option const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 7);
      if(use_adata && !set_metatable(L, "ad_mt_gce_asio_adl.tcp_option")){ luaL_error(L,"unknow type: gce_asio_adl.tcp_option"); }
      {push(L, value.backlog);}
      lua_setfield(L, -2, "backlog");
      {push(L, value.reuse_address);}
      lua_setfield(L, -2, "reuse_address");
      {push(L, value.receive_buffer_size);}
      lua_setfield(L, -2, "receive_buffer_size");
      {push(L, value.send_buffer_size);}
      lua_setfield(L, -2, "send_buffer_size");
      {push(L, value.no_delay);}
      lua_setfield(L, -2, "no_delay");
      {push(L, value.keep_alive);}
      lua_setfield(L, -2, "keep_alive");
      {push(L, value.enable_connection_aborted);}
      lua_setfield(L, -2, "enable_connection_aborted");
    }

  }
}

#endif
