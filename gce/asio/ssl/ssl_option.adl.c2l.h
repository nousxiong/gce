#ifndef ssl_option_adl_cpp2lua_h_
#define ssl_option_adl_cpp2lua_h_

#include <gce/adata/cpp/adata_cpp2lua.hpp>

#include "ssl_option.adl.h"

namespace adata
{
  namespace lua
  {
    ADATA_INLINE void load( lua_State * L, ::gce::asio::adl::ssl_option& value)
    {
      lua_getfield(L, -1, "verify_paths");
      {
        int len = (int)seq_len(L,-1);
        value.verify_paths.resize(len);
        for (int i = 0 ; i < len ; ++i)
        {
          lua_rawgeti(L, -1, i+1);
          {load(L, value.verify_paths[i]);lua_pop(L, 1);}
        }
        lua_pop(L,1);
      }
      lua_getfield(L, -1, "default_verify_paths");
      {load(L, value.default_verify_paths);lua_pop(L, 1);}
      lua_getfield(L, -1, "certificate_authority");
      {load(L, value.certificate_authority);lua_pop(L, 1);}
      lua_getfield(L, -1, "verify_file");
      {load(L, value.verify_file);lua_pop(L, 1);}
      lua_getfield(L, -1, "default_workarounds");
      {load(L, value.default_workarounds);lua_pop(L, 1);}
      lua_getfield(L, -1, "single_dh_use");
      {load(L, value.single_dh_use);lua_pop(L, 1);}
      lua_getfield(L, -1, "no_sslv2");
      {load(L, value.no_sslv2);lua_pop(L, 1);}
      lua_getfield(L, -1, "no_sslv3");
      {load(L, value.no_sslv3);lua_pop(L, 1);}
      lua_getfield(L, -1, "no_tlsv1");
      {load(L, value.no_tlsv1);lua_pop(L, 1);}
      lua_getfield(L, -1, "no_compression");
      {load(L, value.no_compression);lua_pop(L, 1);}
      lua_getfield(L, -1, "verify_depth");
      {load(L, value.verify_depth);lua_pop(L, 1);}
      lua_getfield(L, -1, "verify_none");
      {load(L, value.verify_none);lua_pop(L, 1);}
      lua_getfield(L, -1, "verify_peer");
      {load(L, value.verify_peer);lua_pop(L, 1);}
      lua_getfield(L, -1, "verify_fail_if_no_peer_cert");
      {load(L, value.verify_fail_if_no_peer_cert);lua_pop(L, 1);}
      lua_getfield(L, -1, "verify_client_once");
      {load(L, value.verify_client_once);lua_pop(L, 1);}
      lua_getfield(L, -1, "certificate");
      {load(L, value.certificate);lua_pop(L, 1);}
      lua_getfield(L, -1, "certificate_file");
      {load(L, value.certificate_file);lua_pop(L, 1);}
      lua_getfield(L, -1, "certificate_format");
      {load(L, value.certificate_format);lua_pop(L, 1);}
      lua_getfield(L, -1, "certificate_chain");
      {load(L, value.certificate_chain);lua_pop(L, 1);}
      lua_getfield(L, -1, "certificate_chain_file");
      {load(L, value.certificate_chain_file);lua_pop(L, 1);}
      lua_getfield(L, -1, "private_key");
      {load(L, value.private_key);lua_pop(L, 1);}
      lua_getfield(L, -1, "private_key_file");
      {load(L, value.private_key_file);lua_pop(L, 1);}
      lua_getfield(L, -1, "private_key_format");
      {load(L, value.private_key_format);lua_pop(L, 1);}
      lua_getfield(L, -1, "rsa_private_key");
      {load(L, value.rsa_private_key);lua_pop(L, 1);}
      lua_getfield(L, -1, "rsa_private_key_file");
      {load(L, value.rsa_private_key_file);lua_pop(L, 1);}
      lua_getfield(L, -1, "rsa_private_key_format");
      {load(L, value.rsa_private_key_format);lua_pop(L, 1);}
      lua_getfield(L, -1, "tmp_dh");
      {load(L, value.tmp_dh);lua_pop(L, 1);}
      lua_getfield(L, -1, "tmp_dh_file");
      {load(L, value.tmp_dh_file);lua_pop(L, 1);}
    }

    ADATA_INLINE void push( lua_State * L, ::gce::asio::adl::ssl_option const& value, bool use_adata = true)
    {
      lua_createtable(L, 0, 28);
      if(use_adata && !set_metatable(L, "ad_mt_gce_asio_adl.ssl_option")){ luaL_error(L,"unknow type: gce_asio_adl.ssl_option"); }
      {
        int len = (int)value.verify_paths.size();
        lua_createtable(L, 0, len);
        for (int i = 0 ; i < len ; ++i)
        {
          {push(L, value.verify_paths[i]);}
          lua_rawseti(L, -2, i+1);
        }
      }
      lua_setfield(L, -2, "verify_paths");
      {push(L, value.default_verify_paths);}
      lua_setfield(L, -2, "default_verify_paths");
      {push(L, value.certificate_authority);}
      lua_setfield(L, -2, "certificate_authority");
      {push(L, value.verify_file);}
      lua_setfield(L, -2, "verify_file");
      {push(L, value.default_workarounds);}
      lua_setfield(L, -2, "default_workarounds");
      {push(L, value.single_dh_use);}
      lua_setfield(L, -2, "single_dh_use");
      {push(L, value.no_sslv2);}
      lua_setfield(L, -2, "no_sslv2");
      {push(L, value.no_sslv3);}
      lua_setfield(L, -2, "no_sslv3");
      {push(L, value.no_tlsv1);}
      lua_setfield(L, -2, "no_tlsv1");
      {push(L, value.no_compression);}
      lua_setfield(L, -2, "no_compression");
      {push(L, value.verify_depth);}
      lua_setfield(L, -2, "verify_depth");
      {push(L, value.verify_none);}
      lua_setfield(L, -2, "verify_none");
      {push(L, value.verify_peer);}
      lua_setfield(L, -2, "verify_peer");
      {push(L, value.verify_fail_if_no_peer_cert);}
      lua_setfield(L, -2, "verify_fail_if_no_peer_cert");
      {push(L, value.verify_client_once);}
      lua_setfield(L, -2, "verify_client_once");
      {push(L, value.certificate);}
      lua_setfield(L, -2, "certificate");
      {push(L, value.certificate_file);}
      lua_setfield(L, -2, "certificate_file");
      {push(L, value.certificate_format);}
      lua_setfield(L, -2, "certificate_format");
      {push(L, value.certificate_chain);}
      lua_setfield(L, -2, "certificate_chain");
      {push(L, value.certificate_chain_file);}
      lua_setfield(L, -2, "certificate_chain_file");
      {push(L, value.private_key);}
      lua_setfield(L, -2, "private_key");
      {push(L, value.private_key_file);}
      lua_setfield(L, -2, "private_key_file");
      {push(L, value.private_key_format);}
      lua_setfield(L, -2, "private_key_format");
      {push(L, value.rsa_private_key);}
      lua_setfield(L, -2, "rsa_private_key");
      {push(L, value.rsa_private_key_file);}
      lua_setfield(L, -2, "rsa_private_key_file");
      {push(L, value.rsa_private_key_format);}
      lua_setfield(L, -2, "rsa_private_key_format");
      {push(L, value.tmp_dh);}
      lua_setfield(L, -2, "tmp_dh");
      {push(L, value.tmp_dh_file);}
      lua_setfield(L, -2, "tmp_dh_file");
    }

  }
}

#endif
